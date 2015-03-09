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
#include <FileSystemMessage.h>
#include <FileDescriptor.h>
#include <FileSystem.h>
#include <MemoryMessage.h>
#include "ProcessMessage.h"
#include "ProcessServer.h"

void copyReservedFlags(Address *parentDir, ProcessID child)
{
    MemoryMessage mem;
    ProcessInfo info;
    Address *childDir;

    /* Ask for child information. */
    ProcessCtl(child, InfoPID, (Address) &info);
    
    /* Map page directory of the child. */
    mem.action          = CreatePrivate;
    mem.physicalAddress = info.pageDirectory;
    mem.virtualAddress  = ZERO;
    mem.bytes           = PAGESIZE;
    mem.access          = Memory::Present | Memory::User | Memory::Readable | Memory::Writable;
    mem.ipc(MEMSRV_PID, SendReceive, sizeof(mem));
    
    /* Point to the new mapping. */
    childDir = (Address *) mem.virtualAddress;
    
    /* Copy reserved page entries. */
    for (Size i = 4; i < PAGEDIR_MAX; i++)
    {
	if (parentDir[i] & PAGE_RESERVED)
	{
	    childDir[i] |= PAGE_RESERVED;
	}
    }
    /* Done. */
    mem.action = ReleasePrivate;
    mem.ipc(MEMSRV_PID, SendReceive, sizeof(mem));
}

void ProcessServer::cloneProcessHandler(ProcessMessage *msg)
{
    Address *pageDirectory = (Address *) PAGETABADDR_FROM(PAGETABFROM,
						          PAGEUSERFROM);
    Shared<FileDescriptor> *parentFd, *childFd;
    Address *pageTable;
    FileSystemMessage fs;
    MemoryRange range;
    ProcessID id;
    ProcessInfo info;
    u8 *page;

    /* Create a new Process. */
    id   = ProcessCtl(ANY, Spawn, ZERO);
    page = new u8[PAGESIZE];
    
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
		        VMCopy(msg->from, Read, (Address) page,
			       range.virtualAddress, PAGESIZE);
		
			/* Copy it to the new Process. */
		        VMCopy(id, Write, (Address) page,
			       range.virtualAddress, PAGESIZE);
		    }
		}
	    }
	}
    }
    /* Copy reserved page flags. */
    copyReservedFlags(pageDirectory, id);
    
    /* Inherit strings from parent. */
    strlcpy(procs[id]->command, procs[msg->from]->command, COMMANDLEN);
    strlcpy(procs[id]->currentDirectory,
            procs[msg->from]->currentDirectory, PATHLEN);

    /* Inherit identities. */
    procs[id]->userID  = procs[msg->from]->userID;
    procs[id]->groupID = procs[msg->from]->groupID;
    
    /* Copy the FileDescriptor table. */
    parentFd = getFileDescriptors(files, msg->from);
    childFd  = getFileDescriptors(files, id);
    memcpy(**childFd, **parentFd, childFd->size());
                
    /* Unmap page tables. */
    delete page;
    VMCtl(msg->from, UnMapTables);

    /* Repoint stack of the child. */
    ProcessCtl(msg->from, InfoPID, (Address) &info);
    ProcessCtl(id, SetStack, info.stack);

    /* Begin execution of the child. */
    ProcessCtl(id, Resume);
    
    /* Send a reply to the parent. */
    msg->result = ESUCCESS;
    msg->number = id;
    msg->ipc(msg->from, Send, sizeof(ProcessMessage));
    
    /* And to the child aswell. */
    msg->number = ZERO;
    msg->ipc(id, Send, sizeof(ProcessMessage));
}
