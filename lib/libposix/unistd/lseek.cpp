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
#include <Config.h>
#include "POSIXSupport.h"
#include <errno.h>
#include "unistd.h"

off_t lseek(int fildes, off_t offset, int whence)
{
    FileSystemMessage msg;
    ProcessID mnt = findMount(fildes);
    
    /* Ask for the seek. */
    if (mnt)
    {
	// TODO: use the whence argument.
	msg.action = SeekFile;
	msg.fd     = fildes;
	msg.offset = offset;
	IPCMessage(mnt, SendReceive, &msg, sizeof(msg));
	
	/* Set error number. */
        errno = msg.result;
    }
    else
	errno = ENOENT;
    
    /* Done. */
    return errno == ESUCCESS ? 0 : (off_t) -1;
}
