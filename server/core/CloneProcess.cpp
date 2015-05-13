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
#include <API/VMCopy.h>
#include <API/VMCtl.h>
#include <API/ProcessCtl.h> 
#include <FreeNOS/Memory.h> 
#include <Log.h>
#include "CoreMessage.h"
#include "CoreServer.h"
#include <errno.h>

void CoreServer::copyReservedFlags(ProcessID parent, ProcessID child)
{
    CoreMessage mem;
    ProcessInfo info;
    Address *childDir;

    /* Ask for child information. */
    ProcessCtl(child, InfoPID, (Address) &info);
    
    /* Map page directory of the child. */
    mem.from            = SELF;
    mem.action          = CreatePrivate;
    mem.physicalAddress = info.pageDirectory;
    mem.virtualAddress  = ZERO;
    mem.bytes           = PAGESIZE;
    mem.access          = Memory::Present | Memory::User | Memory::Readable | Memory::Writable;
    createPrivate(&mem);

    /* Point to the new mapping. */
    childDir = (Address *) mem.virtualAddress;
    Address *parentDir = (Address *) PAGETABADDR_FROM(PAGETABFROM, PAGEUSERFROM);

    /* (Re)map the page tables of the parent process. */
    VMCtl(parent, MapTables);
    
    /* Copy reserved page entries. */
    for (Size i = 4; i < PAGEDIR_MAX; i++)
    {
        if (parentDir[i] & PAGE_RESERVED)
        {
            childDir[i] |= PAGE_RESERVED;
        }
    }
    VMCtl(parent, UnMapTables);

    /* Done. */
    mem.action = ReleasePrivate;
    releasePrivate(&mem);
}

void CoreServer::cloneProcessHandler(CoreMessage *msg)
{
    Address *pageDirectory = (Address *) PAGETABADDR_FROM(PAGETABFROM,
						          PAGEUSERFROM);
    Address *pageTable;
    MemoryRange range;
    ProcessID id;
    ProcessInfo info;
    u8 page[PAGESIZE];

    /* Create a new Process. */
    id   = ProcessCtl(ANY, Spawn, ZERO);
    DEBUG("clone: " << procs[msg->from].command << "[" << msg->from << "] => " << id);
    
    /* Map the page tables of the parent process. */
    VMCtl(msg->from, MapTables);

    /* Loop the page directory. */
    for (Size i = 4; i < PAGEDIR_MAX; i++)
    {
	/* Do we need to create a copy this page table (and below)? */
	if (pageDirectory[i] & PAGE_PRESENT)
	{
	    /* Point to the correct page table. */
	    pageTable = PAGETABADDR_FROM(i * PAGESIZE * PAGEDIR_MAX,
                                         PAGEUSERFROM);
	
	    /* Loop the page table. */
	    for (Size j = 0; j < PAGETAB_MAX; j++)
	    {	    
		/* Are we going to create a (hard)copy this page? */
		if (pageTable[j] & PAGE_PRESENT)
		{
		    /* Only allocate a new physical page for non-pinned entries. */
		    if (pageTable[j] & PAGE_PINNED)
		    {
			range.physicalAddress = pageTable[j] & PAGEMASK;
		    }
		    else
			range.physicalAddress = ZERO;
		    
		    /* Calculate virtual address. */
		    range.virtualAddress  = (i * PAGETAB_MAX * PAGESIZE) +
					    (j * PAGESIZE);
		    range.access = getMemoryAccess(pageTable[j] & ~PAGEMASK);
		    
		    /* Perform the mapping. */
		    VMCtl(id, Map, &range);
		    
		    /* Perform hardcopy, if needed. */
		    if (!(pageTable[j] & PAGE_PINNED))
		    {
			/* Copy the page contents of the parent. */
		        VMCopy(msg->from, API::Read, (Address) &page,
			       range.virtualAddress, PAGESIZE);
		
			/* Copy it to the new Process. */
		        VMCopy(id, API::Write, (Address) &page,
			       range.virtualAddress, PAGESIZE);
		    }
		}
	    }
	}
    }
    /* Unmap page tables. */
    VMCtl(msg->from, UnMapTables);

    /* Copy reserved page flags. */
    copyReservedFlags(msg->from, id);
    
    /* Inherit strings from parent. */
    strlcpy(procs[id].command, procs[msg->from].command, COMMANDLEN);

    // Fill UserProcess structure
    procs[id].userID  = procs[msg->from].userID;
    procs[id].groupID = procs[msg->from].groupID;
    procs[id].parent  = msg->from;

    /* Repoint stack of the child. */
    ProcessCtl(msg->from, InfoPID, (Address) &info);
    ProcessCtl(id, SetStack, info.stack);

    /* Begin execution of the child. */
    ProcessCtl(id, Resume);
    
    /* Send a reply to the parent. */
    msg->result = ESUCCESS;
    msg->number = id;
    msg->ipc(msg->from, API::Send, sizeof(CoreMessage));
    
    /* And to the child aswell. */
    msg->number = msg->from;
    msg->result = ECHILD;
    msg->ipc(id, API::Send, sizeof(CoreMessage));
}
