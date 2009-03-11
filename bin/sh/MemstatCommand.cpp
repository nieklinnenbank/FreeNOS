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
#include <MemoryMessage.h>
#include <Config.h>
#include <stdio.h>
#include "MemstatCommand.h"

int MemstatCommand::execute(Size nparams, char **params)
{
    MemoryMessage msg;
        
    /* Query stats. */
    msg.action = MemoryUsage;
    
    /* Ask memory server for memory stats. */
    IPCMessage(MEMSRV_PID, SendReceive, &msg);
    
    /* Print it. */
    printf("Total:     %u KB\n"
           "Available: %u KB\n",
           msg.bytes / 1024, msg.bytesFree / 1024);

    return 0;
}

INITOBJ(MemstatCommand, memstatCmd, NORMAL)
