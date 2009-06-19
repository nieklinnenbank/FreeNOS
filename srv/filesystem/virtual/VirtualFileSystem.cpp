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

#include <API/VMCtl.h>
#include <FreeNOS/BootImage.h>
#include <String.h>
#include "VirtualFileSystem.h"

FileSystemMount    VirtualFileSystem::mounts[MAX_MOUNTS];
UserProcessFSEntry VirtualFileSystem::procs[MAX_PROCS];

VirtualFileSystem::VirtualFileSystem()
    : IPCServer<VirtualFileSystem, FileSystemMessage>(this)
{
    SystemInformation info;
    FileSystemMessage msg;
    BootImage *image = (BootImage *) (0xa1000000);
    String str;

    /* Register message handlers. */
    addIPCHandler(CreateFile,  &VirtualFileSystem::ioHandler, false);
    addIPCHandler(OpenFile,    &VirtualFileSystem::ioHandler, false);
    addIPCHandler(ReadFile,    &VirtualFileSystem::ioHandler, false);
    addIPCHandler(WriteFile,   &VirtualFileSystem::ioHandler, false);
    addIPCHandler(SeekFile,    &VirtualFileSystem::ioHandler, false);
    addIPCHandler(CloseFile,   &VirtualFileSystem::ioHandler, false);
    addIPCHandler(StatFile,    &VirtualFileSystem::ioHandler, false);
    addIPCHandler(IODone,      &VirtualFileSystem::ioDoneHandler, false);
    addIPCHandler(Mount,       &VirtualFileSystem::mountHandler);
    addIPCHandler(MountInfo,   &VirtualFileSystem::mountInfoHandler);
    addIPCHandler(NewProcess,  &VirtualFileSystem::newProcessHandler);
    addIPCHandler(KillProcess, &VirtualFileSystem::killProcessHandler);

    /* Wait for process server to sync BootPrograms from BootImages. */
    for (Size i = 0; i < info.moduleCount; i++)
    {
	/* BootImages have the suffix '.img'. */
	if (str.match(info.modules[i].string, "*.img"))
	{
	    /* Map BootImage into our address space. */
            VMCtl(Map, SELF, info.modules[i].modStart, 0xa1000000);
            VMCtl(Map, SELF, info.modules[i].modStart + PAGESIZE,
                  0xa1000000 + PAGESIZE);

	    /* Loop programs. */
	    for (Size j = 0; j < image->programsTableCount; j++)
	    {
		if (!msg.ipc(PROCSRV_PID, Receive, sizeof(msg)))
		{
		    newProcessHandler(&msg);
		    msg.ipc(PROCSRV_PID, Send, sizeof(msg));
		}
	    }
	}
    }
    /* For now, we only guarantee that /dev is mounted. */
    msg.ipc(DEVSRV_PID, Receive, sizeof(msg));
    mountHandler(&msg);
    msg.ipc(DEVSRV_PID, Send, sizeof(msg));
}

void VirtualFileSystem::ioHandler(FileSystemMessage *msg)
{
    char path[PATHLEN];
    FileSystemMount *mount;
    FileDescriptor *fd;
    ProcessID fsID;

    /* Save the request, so we can reuse it in IODone. */
    msg->savedAction = msg->action;

    /* Fill in the message. */
    msg->userID  = procs[msg->from].userID;
    msg->groupID = procs[msg->from].groupID;
    msg->procID  = msg->from;
    
    /* Lookup mountpoint or filedescriptor. */
    switch (msg->action)
    {
	case CreateFile:
	case OpenFile:
	case StatFile:
    
	    /* Obtain full path first. */
	    if ((msg->result = VMCopy(msg->from, Read, (Address) path,
				     (Address) msg->buffer, PATHLEN)) < 0)
	    {
		msg->error(msg->result);
		return;
	    }
	    /* Retrieve mountpoint. */
	    if (!(mount = findMount(path)))
	    {
		msg->error(ENOENT);
		return;
	    }
	    fsID = mount->procID;
	    break;

	case ReadFile:
	case WriteFile:
	case CloseFile:
	case SeekFile:

	    /* Do we have this fd? */
	    if (!(fd = (*procs[msg->from].files)[msg->fd]))
	    {
		msg->error(ENOENT);
		return;
	    }
	    else if (msg->action != SeekFile)
	    {
		msg->offset = fd->position;
		msg->ident  = fd->identifier;
	    }
	    /* Change file descriptor pointer, for SeekFile. */
	    else
	    {
		fd->position = msg->offset;
		msg->error(ESUCCESS);
		return;
	    }
	    fsID = fd->mount->procID;
	    break;

	default:
	
	    /* No such I/O operation. */
	    msg->error(ENOTSUP);
	    return;
    }
    
    /* Forward the I/O request to the appropriate filesystem server. */
    if (msg->ipc(fsID, Send, sizeof(*msg)) != ESUCCESS)
    {
	ioDoneHandler(msg);
    }
}

void VirtualFileSystem::ioDoneHandler(FileSystemMessage *msg)
{
    FileDescriptor *fd;
    char path[PATHLEN];
    FileSystemMount *mount;
    Error err;

    /* Retrieve correct mountpoint or filedescriptor. */
    switch (msg->savedAction)
    {
	case OpenFile:
	case StatFile:

	    /* Obtain full path first. */
	    if ((err = VMCopy(msg->procID, Read, (Address) path,
				     (Address) msg->buffer, PATHLEN) < 0))
	    {
		msg->error(err, IODone, msg->procID);
		return;
	    }
	    /* Retrieve mountpoint. */
	    if (!(mount = findMount(path)))
	    {
		msg->error(ENOENT, IODone, msg->procID);
		return;
	    }
	    break;

	case ReadFile:
	case WriteFile:
	case CloseFile:
	
	    /* Find the file descriptor. */
	    fd = (*procs[msg->procID].files)[msg->fd];

	default:
	    ;
    }
    /* Post-process the I/O request on success. */
    if (msg->result == ESUCCESS)
    {
	switch (msg->savedAction)
        {
    	    case OpenFile:
	    
	        /* Allocate file descriptor. */
		msg->fd = procs[msg->procID].files->insert(
		    new FileDescriptor(mount, msg->ident));
		procs[msg->procID].fileCount++;
		break;

    	    case CloseFile:
	    
	    	/* Release file. */
		procs[msg->procID].files->remove(msg->fd);
		procs[msg->procID].fileCount--;
		
		/* Free memory. */
		delete fd;
		break;

	    case ReadFile:
	    case WriteFile:

		/* Increment file pointer. */
		fd->position += msg->size;
		
		// TODO: free buffer here, if msg->procID == getPID()!!!
	
	    default:
		;
	}
    }
    /* Inform the user process the I/O operation has finished. */
    msg->ipc(msg->procID, Send, sizeof(*msg));
}

void VirtualFileSystem::mountHandler(FileSystemMessage *msg)
{
    char p[PATHLEN];

    /* Copy the mount point. */
    if ((msg->result = VMCopy(msg->from, Read,
			       (Address)p, (Address)msg->buffer, PATHLEN)))
    {
	insertMount(p, msg->from, ZERO);
    }
}

void VirtualFileSystem::mountInfoHandler(FileSystemMessage *msg)
{
    /* Copy mounts table. */
    if ((msg->size = VMCopy(msg->from, Write,
			   (Address) mounts,
		           (Address) msg->buffer,
			    msg->size < sizeof(mounts) ?
			    msg->size : sizeof(mounts))))
    {
	msg->result = ESUCCESS;
    }
}

void VirtualFileSystem::newProcessHandler(FileSystemMessage *msg)
{
    /* Fill in the new process. */
    procs[msg->procID].userID  = msg->userID;
    procs[msg->procID].groupID = msg->groupID;
    procs[msg->procID].files   = new Array<FileDescriptor>;
    
    /* Success. */
    msg->result = ESUCCESS;
}

void VirtualFileSystem::killProcessHandler(FileSystemMessage *msg)
{
    /* Clear process entry. */
    procs[msg->procID].userID  = 0;
    procs[msg->procID].groupID = 0;
    
    // TODO: don't forget to delete files themselves!!!
    delete procs[msg->procID].files;
    
    /* Success. */
    msg->result = ESUCCESS;
}


void VirtualFileSystem::insertMount(char *path, ProcessID pid, ulong opts)
{
    /* Just scan the entire table for a free entry. */
    for (Size i = 0; i < MAX_MOUNTS; i++)
    {
	if (!mounts[i].path[0])
	{
	    strlcpy(mounts[i].path, path, PATHLEN);
	    mounts[i].procID  = pid;
	    mounts[i].options = opts;
	    break;
	}	
    }
}

FileSystemMount * VirtualFileSystem::findMount(char *path)
{
    /* Find a match. */
    for (Size i = 0; i < MAX_MOUNTS; i++)
    {
	if (mounts[i].path[0])
	{
	    Size len = strlen(mounts[i].path);
    
	    if (strncmp(path, mounts[i].path, len) == 0)
	    {
		return &mounts[i];
	    }
	}
    }
    return (FileSystemMount *) ZERO;
}
