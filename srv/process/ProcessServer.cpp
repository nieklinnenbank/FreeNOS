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
#include <api/SystemInfo.h>                                                       
#include <api/VMCopy.h>                                                           
#include <api/ProcessCtl.h> 
#include <arch/Memory.h> 
#include <FileSystemMessage.h>
#include <stdio.h>
#include "ProcessMessage.h"
#include "ProcessServer.h"

UserProcess ProcessServer::procs[MAX_PROCS];

ProcessServer::ProcessServer()
    : IPCServer<ProcessServer, ProcessMessage>(this)
{
    SystemInformation info;
    FileSystemMessage vfs;

    /* Register message handlers. */
    addIPCHandler(GetID,       &ProcessServer::getIDHandler);
    addIPCHandler(ReadProcess, &ProcessServer::readProcessHandler);
    addIPCHandler(ExitProcess, &ProcessServer::exitProcessHandler, false);

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
    ProcessInfo info;

    /* Find the next process, starting at the given PID. */
    for (Size i = msg->number; i < MAX_PROCS; i++)
    {
	if (procs[i].command[0])
	{
	    /* Request kernel's process information. */
	    ProcessCtl(i, InfoPID, (Address) &info);
	    
	    /* Update entry. */
	    procs[i].state = info.state;
	
	    /* Copy buffer. */
	    VMCopy(msg->from, Write, (Address) (&procs[i]),
				     (Address) (msg->buffer), sizeof(UserProcess));
	    reply->result = ESUCCESS;
	    reply->number = i;
	    return;
	}
    }
    reply->result = ENOSUCH;
}

void ProcessServer::exitProcessHandler(ProcessMessage *msg,
				       ProcessMessage *reply)
{
    printf("PID %u exited with status %d\n",
	    msg->from, msg->number);

    /* Clear process entry. */
    memset(&procs[msg->from], 0, sizeof(UserProcess));

    /* Ask kernel to terminate the process. */
    ProcessCtl(msg->from, KillPID);
}
