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
#include <VirtualFileSystem.h>
#include <Config.h>
#include <stdio.h>
#include "MountCommand.h"

int MountCommand::execute(Size nparams, char **params)
{
    FileSystemMessage msg;
    FileSystemMount mounts[MAX_MOUNTS];
    
    /* Ask filesystem for active mounts. */
    msg.action = MountInfo;
    msg.buffer = (char *) &mounts;
    msg.size   = sizeof(mounts);
        
    /* Trap. */
    IPCMessage(VFSSRV_PID, SendReceive, &msg);

    /* Print out. */
    for (Size i = 0; i < MAX_MOUNTS; i++)
    {
        if (mounts[i].path[0])
            printf("%s\n", mounts[i].path);
    }
    /* Success. */
    return 0;
}

INITOBJ(MountCommand, mountCmd, DEFAULT)
