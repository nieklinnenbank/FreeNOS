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

#include <stdio.h>
#include "ProcessServer.h"

UserProcess ProcessServer::procs[MAX_PROCS];

ProcessServer::ProcessServer()
    : IPCServer<ProcessServer, ProcessMessage>(this)
{
    SystemInformation info;
    FileSystemMessage vfs;

    /* Register message handlers. */
    addIPCHandler(GetID, &ProcessServer::getIDHandler);
    addIPCHandler(ReadProcess, &ProcessServer::readProcessHandler);

    /* Fixup process table, with boot modules. */
    for (Size i = 0; i < info.moduleCount; i++)
    {
	/* Write commandline and identities. */
	snprintf(procs[i].command, COMMANDLEN,
		"[%s]", info.modules[i].string);
	procs[i].uid = 0;
	procs[i].gid = 0;
	
	/* Inform VFS. */
	vfs.newProcess(i, procs[i].uid, procs[i].gid);
    }
    /* Debug out boot modules. */
    printf("Boot Modules:\n");
    
    for (Size i = 0; i < MAX_PROCS; i++)
	if (procs[i].command[0])
	    printf("%u: %s\n", i, procs[i].command);
}

void ProcessServer::getIDHandler(ProcessMessage *msg, ProcessMessage *reply)
{
    reply->number = msg->from;
    reply->result = ESUCCESS;
}

void ProcessServer::readProcessHandler(ProcessMessage *msg, ProcessMessage *reply)
{
    /* Copy only active entries. */
    for (Size i = 0; i < MAX_PROCS; i++)
    {
	if (procs[i].command[0])
	{
	    VMCopy(msg->from, Write, (Address) (&procs[i]),
				     (Address) (msg->buffer + i), sizeof(UserProcess));
	}
    }
    reply->result = ESUCCESS;
}
