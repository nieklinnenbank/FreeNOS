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
#include <API/ProcessCtl.h> 
#include <FreeNOS/Process.h>
#include <Types.h>
#include <ProcessID.h>
#include <Error.h>
#include "ProcessMessage.h"
#include "ProcessServer.h"
#include <string.h>

void ProcessServer::exitProcessHandler(ProcessMessage *msg)
{
    ProcessMessage reply;

    /* Clear process entry. */
    memset(procs[msg->from], 0, sizeof(UserProcess));

    // TODO: close files here!!!

    /* Ask kernel to terminate the process. */
    ProcessCtl(msg->from, KillPID);
    
    /* Awake any processes waiting for this process' death. */
    for (Size i = 0; i < MAX_PROCS; i++)
    {
	if (procs[i]->command[0] &&
	    procs[i]->waitProcessID == msg->from)
	{
	    /* Clear wait status. */
	    procs[i]->waitProcessID = ANY;
	    
	    /* Send exit status. */
	    reply.action = WaitProcess;
	    reply.number = msg->number;
	    reply.result = ESUCCESS;
	    IPCMessage(i, Send, &reply, sizeof(reply));
	}
    }
}
