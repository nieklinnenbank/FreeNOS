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
#include "sys/stat.h"

int stat(const char *path, struct stat *buf)
{
    FileSystemMessage msg;
    FileStat st;
    ProcessID mnt = findMount(path);

    /* Fill message. */
    msg.type   = ChannelMessage::Request;
    msg.action = StatFile;
    msg.path   = (char *) path;
    msg.stat   = &st;

    DEBUG("path = " << (uint) msg.path << " stat = " << (uint) msg.stat);

    /* Ask the FileSystem for the information. */
    if (mnt)
    {
        ChannelClient::instance->syncSendReceive(&msg, mnt);

        // Copy information into buf
        if (msg.result == ESUCCESS)
        {
            buf->fromFileStat(&st);
        }
        // Set errno
        errno = msg.result;
    }
    else
        errno = msg.result = ENOENT;
    
    /* Success. */
    return msg.result == ESUCCESS ? 0 : -1;
}
