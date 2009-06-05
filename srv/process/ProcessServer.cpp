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
#include <api/VMCtl.h>
#include <api/ProcessCtl.h> 
#include <arch/Memory.h> 
#include <arch/BootImage.h>
#include <FileSystemMessage.h>
#include <LogMessage.h>
#include <String.h>
#include <stdio.h>
#include "ProcessMessage.h"
#include "ProcessServer.h"

UserProcess ProcessServer::procs[MAX_PROCS];

ProcessServer::ProcessServer()
    : IPCServer<ProcessServer, ProcessMessage>(this)
{
    SystemInformation info;
    FileSystemMessage vfs;
    BootImage *image = (BootImage *)(0xa1000000);
    BootProgram *program;
    String str;
    Size numProcs = 0;

    /* Register message handlers. */
    addIPCHandler(GetID,       &ProcessServer::getIDHandler);
    addIPCHandler(ReadProcess, &ProcessServer::readProcessHandler);
    addIPCHandler(ExitProcess, &ProcessServer::exitProcessHandler, false);

    /* Fixup process table, with BootPrograms from each BootImage. */
    for (Size i = 0; i < info.moduleCount; i++)
    {
	/* BootImage have the '.img' suffix. */
	if (str.match(info.modules[i].string, "*.img"))
	{
	    /* Map BootImage into our address space. */
	    VMCtl(Map, SELF, info.modules[i].modStart, 0xa1000000);
	    VMCtl(Map, SELF, info.modules[i].modStart + PAGESIZE,
		  0xa1000000 + PAGESIZE);
	    
	    /* Point to the BootProgram table. */
	    program = (BootProgram *)((0xa1000000) + (image->programsTableOffset));
	    
	    /* Loop all embedded programs. */
	    for (Size j = 0; j < image->programsTableCount; j++)
	    {
		/* Write commandline and identities. */
		snprintf(procs[numProcs].command, COMMANDLEN,
			"[%s]", program[j].path);

		/* Set user and group identities. */
	        procs[numProcs].uid = 0;
	        procs[numProcs].gid = 0;
	
		/* Inform VFS. */
		vfs.newProcess(numProcs, procs[numProcs].uid,
					 procs[numProcs].gid);
		numProcs++;
	    }
	}
    }
}

void ProcessServer::getIDHandler(ProcessMessage *msg)
{
    msg->number = msg->from;
    msg->result = ESUCCESS;
}

void ProcessServer::readProcessHandler(ProcessMessage *msg)
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
	    msg->result = ESUCCESS;
	    msg->number = i;
	    return;
	}
    }
    msg->result = ENOENT;
}

void ProcessServer::exitProcessHandler(ProcessMessage *msg)
{
    log("PID %u exited with status %d",
        msg->from, msg->number);

    /* Clear process entry. */
    memset(&procs[msg->from], 0, sizeof(UserProcess));

    /* Ask kernel to terminate the process. */
    ProcessCtl(msg->from, KillPID);
}
