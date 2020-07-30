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

#include <FreeNOS/System.h>
#include <Runtime.h>
#include <ChannelClient.h>
#include <string.h>
#include <unistd.h>
#include "FileSystemMessage.h"
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
    char fullpath[PATHLEN];

    // Use the current directory as prefix for relative paths
    if (path[0] != '/' && m_currentDirectory != NULL)
        snprintf(fullpath, sizeof(fullpath), "%s/%s", **m_currentDirectory, path);
    else
        strlcpy(fullpath, path, sizeof(fullpath));

    msg.path = fullpath;

    if (ChannelClient::instance->syncSendReceive(&msg, sizeof(msg), mnt) == ChannelClient::Success)
    {
        return msg.result;
    }
    else
    {
        return FileSystem::IpcError;
    }
}

FileSystemMount * FileSystemClient::getMounts(Size &numberOfMounts)
{
    numberOfMounts = MaximumFileSystemMounts;
    return m_mounts;
}

ProcessID FileSystemClient::findMount(const char *path) const
{
    FileSystemMount *m = ZERO;
    Size length = 0, len;
    char tmp[PATH_MAX];

    // Is the path relative?
    if (path[0] != '/')
    {
        getcwd(tmp, sizeof(tmp));
        snprintf(tmp, sizeof(tmp), "%s/%s", tmp, path);
    }
    else
        strlcpy(tmp, path, PATH_MAX);

    // Find the longest match
    for (Size i = 0; i < MaximumFileSystemMounts; i++)
    {
        if (m_mounts[i].path[0])
        {
            len = strlen(m_mounts[i].path);

            // Only choose this mount, if it matches,
            // and is longer than the last match.
            if (strncmp(tmp, m_mounts[i].path, len) == 0 && len > length)
            {
                length = len;
                m = &m_mounts[i];
            }
        }
    }

    // All done
    return m ? m->procID : ZERO;
}

FileSystem::Result FileSystemClient::refreshMounts(const char *path)
{
    const ProcessID pid = ProcessCtl(SELF, GetPID);
    Size mountsSize = sizeof(FileSystemMount) * MaximumFileSystemMounts;

    // Skip for rootfs and sysfs
    if (pid == ROOTFS_PID || pid == SYSFS_PID)
        return FileSystem::InvalidArgument;

    // Clear mounts table
    MemoryBlock::set(&m_mounts[2], 0, sizeof(FileSystemMount) * (MaximumFileSystemMounts - 2));

    // Re-read the mounts table from SysFS.
    const ProcessID savedPid = m_pid;
    m_pid = SYSFS_PID;
    const FileSystem::Result result = readFile("/sys/mounts", &m_mounts, &mountsSize, 0U);
    m_pid = savedPid;

    return result;
}

FileSystem::Result FileSystemClient::waitMount(const char *path)
{
    Size len = strlen(path);

    // Send a write containing the requested path to the 'mountwait' file on SysFS
    const ProcessID savedPid = m_pid;
    m_pid = SYSFS_PID;
    const FileSystem::Result result = writeFile("/sys/mountwait", path, &len, 0U);
    m_pid = savedPid;

    return result;
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
                                                const FileSystem::FileModes mode,
                                                const DeviceID deviceId) const
{
    FileSystemMessage msg;
    msg.type     = ChannelMessage::Request;
    msg.action   = FileSystem::CreateFile;
    msg.path     = (char *)path;
    msg.filetype = type;
    msg.mode     = mode;
    msg.deviceID = deviceId;

    return request(path, msg);
}

FileSystem::Result FileSystemClient::readFile(const char *path,
                                              void *buf,
                                              Size *size,
                                              const Size offset) const
{
    FileSystemMessage msg;
    msg.type     = ChannelMessage::Request;
    msg.action   = FileSystem::ReadFile;
    msg.path     = (char *)path;
    msg.buffer   = (char *)buf;
    msg.size     = *size;
    msg.offset   = offset;

    const FileSystem::Result result = request(path, msg);
    if (result == FileSystem::Success)
    {
        *size = msg.size;
    }

    return result;
}

FileSystem::Result FileSystemClient::writeFile(const char *path,
                                               const void *buf,
                                               Size *size,
                                               const Size offset) const
{
    FileSystemMessage msg;
    msg.type     = ChannelMessage::Request;
    msg.action   = FileSystem::WriteFile;
    msg.path     = (char *)path;
    msg.buffer   = (char *)buf;
    msg.size     = *size;
    msg.offset   = offset;

    const FileSystem::Result result = request(path, msg);
    if (result == FileSystem::Success)
    {
        *size = msg.size;
    }

    return result;
}

FileSystem::Result FileSystemClient::statFile(const char *path, FileSystem::FileStat *st) const
{
    FileSystemMessage msg;
    msg.type   = ChannelMessage::Request;
    msg.action = FileSystem::StatFile;
    msg.path   = (char *)path;
    msg.stat   = st;

    return request(path, msg);
}

FileSystem::Result FileSystemClient::deleteFile(const char *path) const
{
    FileSystemMessage msg;
    msg.type   = ChannelMessage::Request;
    msg.action = FileSystem::DeleteFile;
    msg.path   = (char *)path;

    return request(path, msg);
}
