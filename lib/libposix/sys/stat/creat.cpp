/*
 * Copyright (C) 2014 Niek Linnenbank
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
#include <FileSystem.h>
#include "Runtime.h"
#include <errno.h>
#include "sys/stat.h"
#include "unistd.h"
#include "stdio.h"

int creat(const char *path, mode_t mode)
{
    FileSystemMessage msg;
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

    // Fill in the message
    msg.type     = ChannelMessage::Request;
    msg.action   = FileSystem::CreateFile;
    msg.path     = fullpath;
    msg.filetype = FileSystem::RegularFile;
    msg.mode     = (FileSystem::FileModes) (mode & FILEMODE_MASK);

    // Ask FileSystem to create the file for us
    if (mnt)
    {
        ChannelClient::instance->syncSendReceive(&msg, sizeof(msg), mnt);

        // Set errno
        if (msg.result == FileSystem::Success)
            errno = ESUCCESS;
        else
            errno = EIO;
    }
    else
        errno = ENOENT;

    // Report result
    return errno == ESUCCESS ? 0 : -1;
}
