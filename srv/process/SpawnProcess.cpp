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
#include <FileSystem.h>
#include <ExecutableFormat.h>
#include <String.h>
#include <Types.h>
#include <Error.h>
#include "ProcessMessage.h"
#include "ProcessServer.h"
#include <stdio.h>
#include <errno.h>

void ProcessServer::spawnProcessHandler(ProcessMessage *msg)
{
    char path[PATHLEN], *tmp;
    FileSystemMessage fs;
    ExecutableFormat *fmt;
    MemoryRegion regions[16];
    MemoryRange range;
    Error numRegions, ret;
    Size size;
    ProcessID pid;
    Shared<FileDescriptor> *parentFd, *childFd;

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
	    range.virtualAddress  = regions[i].virtualAddress + j;
	    range.physicalAddress = ZERO;
	    range.bytes = PAGESIZE;
	
	    /* Create mapping first. */
	    if ((ret = VMCtl(pid, Map, &range)) != 0)
	    {
		msg->result = ret;
		return;
	    }
	    /* Copy bytes. */
	    VMCopy(pid, Write, (Address) (regions[i].data) + j,
		   regions[i].virtualAddress + j, PAGESIZE);
	}
    }
    /* Create mapping for command-line arguments. */
    range.virtualAddress  = ARGV_ADDR;
    range.physicalAddress = ZERO;
    range.bytes = PAGESIZE;
    VMCtl(pid, Map, &range);

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
    /* Set command-line string. */
    snprintf(procs[pid]->command, COMMANDLEN,
             "%s", path);

    /* Copy the FileDescriptor table. */
    parentFd = getFileDescriptors(files, msg->from);
    childFd  = getFileDescriptors(files, pid);
    memcpy(**childFd, **parentFd, childFd->size());

    /* Inherit user and group identities. */
    procs[pid]->userID  = procs[msg->from]->userID;
    procs[pid]->groupID = procs[msg->from]->groupID;
    strncpy(procs[pid]->currentDirectory, "/", PATHLEN);

    /* Begin execution. */
    ProcessCtl(pid, Resume);

    /* Success. */
    msg->number = pid;
    msg->result = ESUCCESS;
    
    /* Cleanup. */
    delete fmt;
    delete tmp;
}
