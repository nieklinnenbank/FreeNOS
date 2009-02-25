/**
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

#include "MemoryServer.h"

MemoryServer::MemoryServer()
{
    for (Size i = 0; i < MAX_PROCS; i++)
    {
	heaps[i] = HEAP_START;
    }
}
	
int MemoryServer::run()
{
    MemoryMessage msg, reply;

    /* Enter loop. */
    while (true)
    {
	/* Now wait for a message. */
	IPCMessage(ZERO, Receive, &msg);
	
	/* Handle incoming request. */
	switch (msg.action)
	{
	    case HeapGrow:
		doGrow(&msg, &reply);
		break;
	
	    case HeapShrink:
		doShrink(&msg, &reply);
		break;
	
	    case MemoryUsage:
		doUsage(&msg, &reply);
		break;
	
	    default:
		continue;
	}
	/* Fill in reply. */
	reply.startAddr = HEAP_START;
	reply.endAddr   = heaps[msg.from];
	
	/* Send reply. */
	IPCMessage(msg.from, Send, &reply);
    }
    /* Satify compiler. */
    return 0;
}

void MemoryServer::doGrow(MemoryMessage *msg, MemoryMessage *reply)
{
    Size num = 0;

    /* Allocate virtual memory pages. */
    while (num < msg->bytes && heaps[msg->from] < HEAP_END)
    {
	VMCtl(msg->from, ZERO, heaps[msg->from]);
	heaps[msg->from] += PAGESIZE;
	num += PAGESIZE;
    }
    /* Update reply. */
    reply->bytes  = num;
    reply->action = MemoryOK;
}

void MemoryServer::doShrink(MemoryMessage *msg, MemoryMessage *reply)
{
    Size num;
    
    /* Release virtual memory pages. */
    while (num < msg->bytes && heaps[msg->from] > HEAP_END)
    {
	VMCtl(msg->from, ZERO, heaps[msg->from] - MEMALIGN, ZERO);
	heaps[msg->from] -= PAGESIZE;
	num += PAGESIZE;
    }
    /* Update reply. */
    reply->bytes  = num;
    reply->action = MemoryOK;
}

void MemoryServer::doUsage(MemoryMessage *msg, MemoryMessage *reply)
{
    SystemInformation info;

    /* Retrieve system information. */
    SystemInfo(&info);
    
    /* Fill in the reply. */
    reply->bytes     = info.memorySize;
    reply->bytesFree = info.memoryAvail;
}
