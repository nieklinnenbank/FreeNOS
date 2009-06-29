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
#include <API/SystemInfo.h>                                                       
#include <API/VMCopy.h>
#include <API/VMCtl.h>
#include <API/ProcessCtl.h> 
#include <FreeNOS/Memory.h> 
#include <FreeNOS/BootImage.h>
#include <FileSystemMessage.h>
#include <FileSystem.h>
#include <LogMessage.h>
#include <MemoryMessage.h>
#include <ExecutableFormat.h>
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
    addIPCHandler(GetID,        &ProcessServer::getIDHandler);
    addIPCHandler(ReadProcess,  &ProcessServer::readProcessHandler);
    addIPCHandler(ExitProcess,  &ProcessServer::exitProcessHandler,  false);
    addIPCHandler(SpawnProcess, &ProcessServer::spawnProcessHandler);
    addIPCHandler(CloneProcess, &ProcessServer::cloneProcessHandler, false);
    addIPCHandler(WaitProcess,  &ProcessServer::waitProcessHandler,  false);

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
		vfs.newProcess(numProcs, ANY,
			       procs[numProcs].uid,
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
    MemoryMessage mem;
    FileSystemMessage vfs;
    ProcessMessage reply;

    /* Clear process entry. */
    memset(&procs[msg->from], 0, sizeof(UserProcess));

    /* Ask kernel to terminate the process. */
    ProcessCtl(msg->from, KillPID);
    
    /* Inform memory server. */
    mem.action = HeapReset;
    mem.pid    = msg->from;
    IPCMessage(MEMSRV_PID, SendReceive, &mem, sizeof(mem));
    
    /* Awake any processes waiting for this process' death. */
    for (Size i = 0; i < MAX_PROCS; i++)
    {
	if (procs[i].command[0] &&
	    procs[i].waitProcessID == msg->from)
	{
	    /* Clear wait status. */
	    procs[i].waitProcessID = ANY;
	    
	    /* Send exit status. */
	    reply.action = WaitProcess;
	    reply.number = msg->number;
	    reply.result = ESUCCESS;
	    IPCMessage(i, Send, &reply, sizeof(reply));
	}
    }
}

void ProcessServer::spawnProcessHandler(ProcessMessage *msg)
{
    char path[PATHLEN], *tmp;
    FileSystemMessage fs;
    ExecutableFormat *fmt;
    MemoryRegion regions[16];
    Error numRegions, ret;
    Size size;
    ProcessID pid;

    /* Read out the path to the executable. */
    if ((msg->result = VMCopy(msg->from, Read, (Address) path,
                             (Address) msg->path, PATHLEN) < 0))
    {
        return;
    }
    /* Attempt to read executable format. */
    if (!(fmt = ExecutableFormat::find(path)))
    {
	msg->result = errno;
	return;
    }
    /* Retrieve memory regions. */
    if ((numRegions = fmt->regions(regions, 16)) < 0)
    {
	msg->result = errno;
	return;
    }
    /* Create new process. */
    pid = ProcessCtl(ANY, Spawn, fmt->entry());

    /* Map program regions into virtual memory of the new process. */
    for (int i = 0; i < numRegions; i++)
    {
	/* Copy executable memory from this region. */
	for (Size j = 0; j < regions[i].size; j += PAGESIZE)
	{
	    /* Create mapping first. */
	    if ((ret = VMCtl(Map, pid, ZERO,
			     regions[i].virtualAddress + j)) != 0)
	    {
		msg->result = ret;
		return;
	    }
	    /* Copy bytes. */
	    VMCopy(pid, Write, (Address) (regions[i].data) + j,
		   regions[i].virtualAddress + j, PAGESIZE);
	}
    }
    /* Set command-line string. */
    snprintf(procs[pid].command, COMMANDLEN,
             "%s", path);

    /* Create mapping for command-line arguments. */
    VMCtl(Map, pid, ZERO, ARGV_ADDR);

    /* Allocate temporary variable. */
    tmp  = new char[PAGESIZE];
    memset(tmp, 0, PAGESIZE);

    /* Calculate number of bytes to copy. */
    size = msg->number * ARGV_SIZE < PAGESIZE ?
	   msg->number * ARGV_SIZE : PAGESIZE;

    /* Copy arguments into the temporary variable. */    
    if ((msg->result = VMCopy(msg->from, Read, (Address) tmp,
			     (Address) msg->arguments, size)) < 0)
    {
	delete tmp;
	return;
    }
    /* Copy argc/argv into the new process. */
    if ((msg->result = VMCopy(pid, Write, (Address) tmp,
		    	     (Address) ARGV_ADDR, PAGESIZE)) < 0)
    {
	delete tmp;
	return;
    }
    /* Inherit user and group identities. */
    procs[pid].uid = procs[msg->from].uid;
    procs[pid].gid = procs[msg->from].gid;

    /* Inform VFS. */
    fs.newProcess(pid, msg->from,
		  procs[pid].uid,
    		  procs[pid].gid);

    /* Begin execution. */
    ProcessCtl(pid, Resume);

    /* Success. */
    msg->number = pid;
    msg->result = ESUCCESS;
    
    /* Cleanup. */
    delete fmt;
    delete tmp;
}

void ProcessServer::cloneProcessHandler(ProcessMessage *msg)
{
    FileSystemMessage fs;
    MemoryMessage mem;
    ProcessID id;
    Address  pageAddress   = PAGEUSERFROM;
    Address *pageDirectory = (Address *) PAGETABADDR_FROM(pageAddress, pageAddress);
    Address *pageTable;
    Address  vaddr;
    ProcessInfo info;
    u8 *page;

    /* Create a new Process. */
    id   = ProcessCtl(ANY, Spawn, ZERO);
    page = new u8[PAGESIZE];
    
    /* Map the page tables of the parent process. */
    VMCtl(MapTables, msg->from);

    /* Loop the page directory. */
    for (Size i = 4; i < PAGEDIR_MAX; i++)
    {
	/* Do we need to create a copy this page table (and below)? */
	if ((pageDirectory[i] & PAGE_PRESENT) &&
	   !(pageDirectory[i] & PAGE_PINNED))
	{
	    /* Point to the correct page table. */
	    pageTable = PAGETABADDR_FROM(i * PAGESIZE * PAGEDIR_MAX,
                                         pageAddress);
	
	    /* Loop the page table. */
	    for (Size j = 0; j < PAGETAB_MAX; j++)
	    {	    
		/* Are we going to create a copy this page? */
		if ((pageTable[j] & PAGE_PRESENT) &&
		   !(pageTable[j] & PAGE_PINNED))
		{
		    /* Calculate virtual address. */
		    vaddr = (i * PAGETAB_MAX * PAGESIZE) +
			    (j * PAGESIZE);
		    
		    /* Allocate a physical page. */
		    VMCtl(Map, id, ZERO, vaddr);
		    
		    /* Copy the page contents of the parent. */
		    VMCopy(msg->from, Read, (Address) page,
			  (Address) vaddr, PAGESIZE);
		
		    /* Copy it to the new Process. */
		    VMCopy(id, Write, (Address) page,
			  (Address) vaddr, PAGESIZE);
		}
	    }
	}
    }
    /* Inherit user and group identities. */
    strlcpy(procs[id].command, procs[msg->from].command, COMMANDLEN);
    procs[id].uid = procs[msg->from].uid;
    procs[id].gid = procs[msg->from].gid;
                
    /* Inform VFS. */
    fs.newProcess(id, msg->from,
                  procs[id].uid,
                  procs[id].gid);

    /* Unmap page tables. */
    delete page;
    VMCtl(UnMapTables, msg->from, ZERO, pageAddress);

    /* Repoint stack of the child. */
    ProcessCtl(msg->from, InfoPID, (Address) &info);
    ProcessCtl(id, SetStack, info.stack);

    /* Repoint heap of the child. */
    mem.action = HeapClone;
    mem.pid    = id;
    mem.ppid   = msg->from;
    IPCMessage(MEMSRV_PID, SendReceive, &mem, sizeof(MemoryMessage));

    /* Begin execution of the child. */
    ProcessCtl(id, Resume);
    
    /* Send a reply to the parent. */
    msg->result = ESUCCESS;
    msg->number = id;
    msg->ipc(msg->from, Send, sizeof(ProcessMessage));
    
    /* And to the child aswel. */
    msg->number = ZERO;
    msg->ipc(id, Send, sizeof(ProcessMessage));
}

void ProcessServer::waitProcessHandler(ProcessMessage *msg)
{
    if (msg->number < MAX_PROCS && msg->number != ANY &&
	procs[msg->number].command[0])
    {
	procs[msg->from].waitProcessID = msg->number;
    }
    else
    {
	msg->result = EINVAL;
	IPCMessage(msg->from, Send, msg, sizeof(*msg));
    }
}
