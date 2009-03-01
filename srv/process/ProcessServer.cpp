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

#include "ProcessServer.h"
#include "MemoryServer.h"
#include <stdio.h>

ProcessServer::ProcessServer()
    : IPCServer<ProcessServer, ProcessMessage>(this), procs((UserProcess *)PROCTABLE)
{
    /* Register message handlers. */
    addIPCHandler(GetID, &ProcessServer::doGetID);
    
    /* Debug out boot modules. */
    printf("Boot Modules:\n");
    
    for (Size i = 0; i < MAX_PROCS; i++)
	if (procs[i].command[0])
	    printf("%u: %s\n", i, procs[i].command);
}

void ProcessServer::doGetID(ProcessMessage *msg, ProcessMessage *reply)
{
    reply->number = msg->from;
    reply->action = ProcessOK;
}
