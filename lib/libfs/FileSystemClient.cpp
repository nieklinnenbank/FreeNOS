/*
 * Copyright (C) 2020 Niek Linnenbank
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Log.h>
#include <ChannelClient.h>
#include <KernelTimer.h>
#include "FileSystemMessage.h"
#include "FileDescriptor.h"
#include "FileSystemClient.h"

FileSystemMount FileSystemClient::m_mounts[MaximumFileSystemMounts] = {};

String * FileSystemClient::m_currentDirectory = (String *) NULL;

FileSystemClient::FileSystemClient(const ProcessID pid)
    : m_pid(pid)
{
}

inline FileSystem::Result FileSystemClient::request(const char *path,
                                                    FileSystemMessage &msg) const
{
    const ProcessID mnt = m_pid == ANY ? findMount(path) : m_pid;
    char fullpath[FileSystemPath::MaximumLength];

    // Use the current directory as prefix for relative paths
    if (path[0] != '/' && m_currentDirectory != NULL)
    {
        const Size copied = MemoryBlock::copy(fullpath, **m_currentDirectory, sizeof(fullpath));

        if (copied < sizeof(fullpath))
            MemoryBlock::copy(fullpath + copied, path, sizeof(fullpath) - copied);
    }
    else
    {
        MemoryBlock::copy(fullpath, path, sizeof(fullpath));
    }

    msg.buffer = fullpath;

    return request(mnt, msg);
}

inline FileSystem::Result FileSystemClient::request(const ProcessID pid,
                                                    FileSystemMessage &msg) const
{
    ChannelClient::Result r = ChannelClient::instance()->syncSendReceive(&msg, sizeof(msg), pid);
    if (r != ChannelClient::Success)
    {
        ERROR("failed to send request to PID " << pid <<
              " for path " << msg.buffer << ": result = " << (int) r);
        return FileSystem::IpcError;
    }
    else if (msg.result != FileSystem::RedirectRequest)
    {
        return msg.result;
    }

    // If the path is mounted by a different file system process, the request is re-directed.
    // Update the cached file system mounts table and re-send the request.
    assert(msg.pid != ROOTFS_PID);
    assert(msg.action != FileSystem::ReadFile);
    assert(msg.action != FileSystem::WriteFile);

    // Extend mounts table
    for (Size i = 0; i < MaximumFileSystemMounts; i++)
    {
        if (m_mounts[i].path[0] == ZERO)
        {
            assert(msg.pathMountLength + 1 <= sizeof(m_mounts[i].path));
            MemoryBlock::copy(m_mounts[i].path, msg.buffer, msg.pathMountLength + 1);
            m_mounts[i].procID  = msg.pid;
            m_mounts[i].options = ZERO;
            break;
        }
    }

    msg.type = ChannelMessage::Request;
    r = ChannelClient::instance()->syncSendReceive(&msg, sizeof(msg), msg.pid);
    if (r != ChannelClient::Success)
    {
        ERROR("failed to redirect request to PID " << msg.pid <<
              " for path " << msg.buffer << ": result = " << (int) r);
        return FileSystem::IpcError;
    }

    assert (msg.result != FileSystem::RedirectRequest);
    return msg.result;
}

ProcessID FileSystemClient::findMount(const char *path) const
{
    FileSystemMount *m = ZERO;
    Size length = 0;
    char fullpath[FileSystemPath::MaximumLength];

    // Use the current directory as prefix for relative paths
    if (path[0] != '/' && m_currentDirectory != NULL)
    {
        const Size copied = MemoryBlock::copy(fullpath, **m_currentDirectory, sizeof(fullpath));

        if (copied < sizeof(fullpath))
            MemoryBlock::copy(fullpath + copied, path, sizeof(fullpath) - copied);
    }
    else
    {
        MemoryBlock::copy(fullpath, path, sizeof(fullpath));
    }

    // Find the longest match
    for (Size i = 0; i < MaximumFileSystemMounts; i++)
    {
        if (m_mounts[i].path[0])
        {
            String str(m_mounts[i].path, false);
            Size len = str.length();

            // Only choose this mount, if it matches,
            // and is longer than the last match.
            if (str.compareTo(fullpath, true, len) == 0 && len > length)
            {
                length = len;
                m = &m_mounts[i];
            }
        }
    }

    // All done
    return m ? m->procID : ROOTFS_PID;
}

const String * FileSystemClient::getCurrentDirectory() const
{
    return m_currentDirectory;
}

void FileSystemClient::setCurrentDirectory(const String &directory)
{
    assert(m_currentDirectory != NULL);
    *m_currentDirectory = directory;
}

void FileSystemClient::setCurrentDirectory(String *directory)
{
    if (m_currentDirectory != NULL)
    {
        *m_currentDirectory = *directory;
    }
    else
    {
        m_currentDirectory = directory;
    }
}

FileSystem::Result FileSystemClient::createFile(const char *path,
                                                const FileSystem::FileType type,
                                                const FileSystem::FileModes mode) const
{
    FileSystem::FileStat st;
    st.type    = type;
    st.access  = mode;

    FileSystemMessage msg;
    msg.type   = ChannelMessage::Request;
    msg.action = FileSystem::CreateFile;
    msg.buffer = (char *)path;
    msg.stat   = &st;

    return request(path, msg);
}

FileSystem::Result FileSystemClient::statFile(const char *path,
                                              FileSystem::FileStat *st) const
{
    FileSystemMessage msg;
    msg.type   = ChannelMessage::Request;
    msg.action = FileSystem::StatFile;
    msg.buffer = (char *)path;
    msg.stat   = st;

    return request(path, msg);
}

FileSystem::Result FileSystemClient::openFile(const char *path,
                                              Size & descriptor) const
{
    FileSystem::FileStat st;

    const FileSystem::Result result = statFile(path, &st);
    if (result == FileSystem::Success)
    {
        FileDescriptor *fd = FileDescriptor::instance();

        const FileDescriptor::Result fdResult = fd->openEntry(st.inode, st.pid, descriptor);
        if (fdResult != FileDescriptor::Success)
        {
            return FileSystem::IOError;
        }
    }

    return result;
}

FileSystem::Result FileSystemClient::closeFile(const Size descriptor) const
{
    const FileDescriptor::Result result = FileDescriptor::instance()->closeEntry(descriptor);
    if (result != FileDescriptor::Success)
    {
        return FileSystem::IOError;
    }

    return FileSystem::Success;
}

FileSystem::Result FileSystemClient::readFile(const Size descriptor,
                                              void *buf,
                                              Size *size) const
{
    FileDescriptor::Entry *fd = FileDescriptor::instance()->getEntry(descriptor);
    if (!fd || !fd->open)
    {
        return FileSystem::NotFound;
    }

    FileSystemMessage msg;
    msg.type     = ChannelMessage::Request;
    msg.action   = FileSystem::ReadFile;
    msg.inode    = fd->inode;
    msg.buffer   = (char *)buf;
    msg.size     = *size;
    msg.offset   = fd->position;

    const FileSystem::Result result = request(fd->pid, msg);
    if (result == FileSystem::Success)
    {
        *size = msg.size;
        fd->position += msg.size;
    }

    return result;
}

FileSystem::Result FileSystemClient::writeFile(const Size descriptor,
                                               const void *buf,
                                               Size *size) const
{
    FileDescriptor::Entry *fd = FileDescriptor::instance()->getEntry(descriptor);
    if (!fd || !fd->open)
    {
        return FileSystem::NotFound;
    }

    FileSystemMessage msg;
    msg.type     = ChannelMessage::Request;
    msg.action   = FileSystem::WriteFile;
    msg.inode    = fd->inode;
    msg.buffer   = (char *)buf;
    msg.size     = *size;
    msg.offset   = fd->position;

    const FileSystem::Result result = request(fd->pid, msg);
    if (result == FileSystem::Success)
    {
        *size = msg.size;
        fd->position += msg.size;
    }

    return result;
}


FileSystem::Result FileSystemClient::deleteFile(const char *path) const
{
    FileSystemMessage msg;
    msg.type   = ChannelMessage::Request;
    msg.action = FileSystem::DeleteFile;
    msg.buffer = (char *)path;

    return request(path, msg);
}

FileSystem::Result FileSystemClient::waitFile(const char *filesystemPath,
                                              const FileSystem::WaitSet *waitSet,
                                              const Size count,
                                              const Size msecTimeout) const
{
    const ProcessID pid = findMount(filesystemPath);

    FileSystemMessage msg;
    msg.type = ChannelMessage::Request;
    msg.action = FileSystem::WaitFile;
    msg.buffer = (char *) waitSet;
    msg.size = count * sizeof(FileSystem::WaitSet);

    if (msecTimeout != 0)
    {
        KernelTimer timer;
        timer.tick();
        timer.getCurrent(&msg.timeout, 500);
    }
    else
    {
        msg.timeout.ticks = 0;
        msg.timeout.frequency = 0;
    }

    return request(pid, msg);
}

FileSystem::Result FileSystemClient::mountFileSystem(const char *mountPath) const
{
    FileSystemMessage msg;
    msg.type   = ChannelMessage::Request;
    msg.action = FileSystem::MountFileSystem;
    msg.buffer = (char *) mountPath;

    return request(ROOTFS_PID, msg);
}

FileSystem::Result FileSystemClient::waitFileSystem(const char *path) const
{
    FileSystemMessage msg;
    msg.type   = ChannelMessage::Request;
    msg.action = FileSystem::WaitFileSystem;
    msg.buffer = (char *) path;

    return request(ROOTFS_PID, msg);
}

FileSystemMount * FileSystemClient::getFileSystems(Size &numberOfMounts) const
{
    FileSystemMessage msg;
    msg.type   = ChannelMessage::Request;
    msg.action = FileSystem::GetFileSystems;
    msg.buffer = (char *) m_mounts;
    msg.size   = sizeof(m_mounts);

    const FileSystem::Result result = request(ROOTFS_PID, msg);
    if (result == FileSystem::Success)
    {
        numberOfMounts = MaximumFileSystemMounts;
        return m_mounts;
    }

    return (FileSystemMount *) NULL;
}
