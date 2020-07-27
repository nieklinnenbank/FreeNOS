/*
 * Copyright (C) 2009 Niek Linnenbank
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
#include <FileSystemMessage.h>
#include "Runtime.h"
#include <errno.h>
#include <Log.h>
#include "unistd.h"
#include "sys/stat.h"

int stat(const char *path, struct stat *buf)
{
    FileSystemMessage msg;
    FileStat st;
    ProcessID mnt = findMount(path);
    char fullpath[PATH_MAX];

    // Relative or absolute?
    if (path[0] != '/')
    {
        char cwd[PATH_MAX];

        // What's the current working dir?
        getcwd(cwd, PATH_MAX);
        snprintf(fullpath, sizeof(fullpath), "%s/%s", cwd, path);
    }
    else
        strlcpy(fullpath, path, sizeof(fullpath));

    // Fill message
    msg.type   = ChannelMessage::Request;
    msg.action = FileSystem::StatFile;
    msg.path   = fullpath;
    msg.stat   = &st;

    DEBUG("path = " << (void *) msg.path << " stat = " << (void *) msg.stat);

    // Ask the FileSystem for the information
    if (mnt)
    {
        ChannelClient::instance->syncSendReceive(&msg, sizeof(msg), mnt);

        // Copy information into buf
        if (msg.result == FileSystem::Success)
        {
            buf->fromFileStat(&st);
            errno = ESUCCESS;
        }
        else
        {
            errno = EIO;
        }
    }
    else
    {
        errno = ENOENT;
    }

    // Success
    return errno == ESUCCESS ? 0 : -1;
}
