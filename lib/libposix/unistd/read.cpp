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

#include <API/IPCMessage.h>
#include <FileSystemMessage.h>
#include "POSIXSupport.h"
#include <Config.h>
#include <errno.h>
#include "unistd.h"

ssize_t read(int fildes, void *buf, size_t nbyte)
{
    FileSystemMessage msg;
    ProcessID mnt = findMount(fildes);

    /* Read the file. */
    if (mnt)
    {
	msg.action = ReadFile;
        msg.fd     = fildes;
        msg.buffer = (char *) buf;
        msg.size   = nbyte;
        msg.offset = ZERO;
        IPCMessage(mnt, SendReceive, &msg, sizeof(msg));

        /* Set error number. */
	errno = msg.result;
    }
    else
	errno = ENOENT;
    
    /* Success. */
    return errno >= 0 ? errno : (ssize_t) -1;
}
