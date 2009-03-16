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

#include <api/IPCMessage.h>
#include <FileSystemMessage.h>
#include <Config.h>
#include <errno.h>
#include "stat.h"

int stat(const char *path, struct stat *buf)
{
    FileSystemMessage msg;

    /* Fill message. */
    msg.action = StatFile;
    msg.buffer = (char *) path;
    msg.stat   = buf;
    
    /* Ask VFS for the information. */
    IPCMessage(VFSSRV_PID, SendReceive, &msg);

    /* Set errno. */
    errno = msg.result;
    
    /* Success. */
    return msg.result == ESUCCESS ? 0 : -1;
}

int mknod(const char *path, mode_t mode, dev_t dev)
{
    FileSystemMessage msg;
    
    /* Fill in the message. */
    msg.action   = CreateFile;
    msg.buffer   = (char *) path;
    msg.deviceID = dev;
    msg.filetype = CharacterDeviceFile;
    msg.mode     = mode;
    
    /* Ask VFS to create the file for us. */
    IPCMessage(VFSSRV_PID, SendReceive, &msg);
    
    /* Set errno. */
    errno = msg.result;

    /* Report result. */
    return msg.result == ESUCCESS ? 0 : -1;
}
