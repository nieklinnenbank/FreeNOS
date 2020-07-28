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

#include <Runtime.h>
#include <ChannelClient.h>
#include "FileSystemMessage.h"
#include "FileSystemClient.h"

FileSystemClient::FileSystemClient(const ProcessID pid)
    : m_pid(pid)
{
}

inline FileSystem::Result FileSystemClient::request(const char *path,
                                                    FileSystemMessage &msg) const
{
    const ProcessID mnt = m_pid == ANY ? findMount(path) : m_pid;

    if (ChannelClient::instance->syncSendReceive(&msg, sizeof(msg), mnt) == ChannelClient::Success)
    {
        return msg.result;
    }
    else
    {
        return FileSystem::IpcError;
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
