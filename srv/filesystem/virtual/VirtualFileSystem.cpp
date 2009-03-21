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

#include "VirtualFileSystem.h"

FileSystemMount    VirtualFileSystem::mounts[MAX_MOUNTS];
UserProcessFSEntry VirtualFileSystem::procs[MAX_PROCS];

VirtualFileSystem::VirtualFileSystem()
    : IPCServer<VirtualFileSystem, FileSystemMessage>(this)
{
    SystemInformation info;
    FileSystemMessage msg;

    /* Register message handlers. */
    addIPCHandler(CreateFile,  &VirtualFileSystem::createFileHandler);
    addIPCHandler(OpenFile,    &VirtualFileSystem::openFileHandler);
    addIPCHandler(ReadFile,    &VirtualFileSystem::readFileHandler, false);
    addIPCHandler(CloseFile,   &VirtualFileSystem::closeFileHandler);
    addIPCHandler(StatFile,    &VirtualFileSystem::statFileHandler);
    addIPCHandler(Mount,       &VirtualFileSystem::mountHandler);
    addIPCHandler(MountInfo,   &VirtualFileSystem::mountInfoHandler);
    addIPCHandler(NewProcess,  &VirtualFileSystem::newProcessHandler);
    addIPCHandler(KillProcess, &VirtualFileSystem::killProcessHandler);
    addIPCHandler(IODone,      &VirtualFileSystem::ioDoneHandler, false);
    
    /* Wait for process server to sync procs. */
    for (Size i = 0; i < info.moduleCount; i++)
    {
	if (!msg.ipc(PROCSRV_PID, Receive, sizeof(msg)))
	{
	    newProcessHandler(&msg, &msg);
	    msg.ipc(PROCSRV_PID, Send, sizeof(msg));
	}
    }
    /* For now, we only guarantee that /dev is mounted. */
    msg.ipc(DEVSRV_PID, Receive, sizeof(msg));
    mountHandler(&msg, &msg);
    msg.ipc(DEVSRV_PID, Send, sizeof(msg));
}

void VirtualFileSystem::createFileHandler(FileSystemMessage *msg,
					  FileSystemMessage *reply)
{
    char path[PATHLEN];
    FileSystemMount *mount;

    /* Obtain full path first. */
    if ((reply->result = VMCopy(msg->from, Read, (Address) path,
			       (Address) msg->buffer, PATHLEN) < 0))
    {
    	return;
    }
    /* Retrieve mountpoint. */
    if (!(mount = findMount(path)))
    {
	reply->result = ENOSUCH;
	return;
    }
    /* Ask the filesystem server to create the given file. */
    reply->userID  = procs[msg->from].userID;
    reply->groupID = procs[msg->from].groupID;
    reply->createFile(path + strlen(mount->path), msg->filetype,
		      msg->mode, msg->deviceID.major, msg->deviceID.minor,
		      mount->procID);
}

void VirtualFileSystem::openFileHandler(FileSystemMessage *msg,
					FileSystemMessage *reply)
{
    char path[PATHLEN];
    FileSystemMount *mount;
    
    /* Obtain full path first. */
    if ((reply->result = VMCopy(msg->from, Read, (Address) path,
			       (Address) msg->buffer, PATHLEN) < 0))
    {
    	return;
    }
    /* Retrieve mountpoint. */
    if (!(mount = findMount(path)))
    {
	reply->result = ENOSUCH;
	return;
    }
    /* Ask filesystem server to open it. */
    reply->openFile(path + strlen(mount->path), mount->procID);
    
    /* Allocate file descriptor. */
    if (reply->result == ESUCCESS)
    {
	reply->fd = procs[msg->from].files->insert(new FileDescriptor(mount, reply->ident));
        procs[msg->from].fileCount++;
    }
}

void VirtualFileSystem::readFileHandler(FileSystemMessage *msg,
					FileSystemMessage *reply)
{
    FileDescriptor *fd;
    
    /* Do we have this fd? */
    if (!(fd = (*procs[msg->from].files)[msg->fd]))
    {
	reply->result = ENOSUCH;
	return;
    }
    /* Request (asynchroneous) read operation. */
    reply->action = ReadFile;
    reply->buffer = msg->buffer;
    reply->size   = msg->size;
    reply->offset = fd->position;
    reply->fd     = msg->fd;
    reply->ident  = fd->identifier;
    reply->procID = msg->from;
    reply->ipc(fd->mount->procID, Send, sizeof(*reply));
}

void VirtualFileSystem::closeFileHandler(FileSystemMessage *msg,
					 FileSystemMessage *reply)
{
    FileDescriptor *fd;

    /* Do we have it opened? */
    if ((fd = (*procs[msg->from].files)[msg->fd]))
    {
	/* Inform filesystem we are closing the file. */
	reply->ident  = fd->identifier;
	reply->procID = msg->from;
	reply->closeFile(msg->fd, fd->mount->procID);
	
	/* Release file. */
	procs[msg->from].files->remove(msg->fd);
	procs[msg->from].fileCount--;
	
	/* Free memory. */
	delete fd;
    }
    else
	reply->result = ENOSUCH;
}

void VirtualFileSystem::statFileHandler(FileSystemMessage *msg,
					FileSystemMessage *reply)
{
    char path[PATHLEN];
    FileSystemMount *mount;
    FileSystemMessage fs;
    
    /* Obtain full path first. */
    if ((reply->result = VMCopy(msg->from, Read, (Address) path,
			       (Address) msg->buffer, PATHLEN) < 0))
    {
    	return;
    }
    /* Retrieve mountpoint. */
    if (!(mount = findMount(path)))
    {
	reply->result = ENOSUCH;
	return;
    }
    /* Now we stat the file. */
    fs.action = StatFile;
    fs.buffer = path + strlen(mount->path);
    fs.stat   = msg->stat;    
    fs.procID = msg->from;
    IPCMessage(mount->procID, SendReceive, &fs, sizeof(fs));

    /* Report results. */
    reply->result = fs.result;
}

void VirtualFileSystem::mountHandler(FileSystemMessage *msg,
				     FileSystemMessage *reply)
{
    char p[PATHLEN];

    /* Copy the mount point. */
    if ((reply->result = VMCopy(msg->from, Read,
			       (Address)p, (Address)msg->buffer, PATHLEN)))
    {
	insertMount(p, msg->from, ZERO);
    }
}

void VirtualFileSystem::mountInfoHandler(FileSystemMessage *msg,
					 FileSystemMessage *reply)
{
    /* Copy mounts table. */
    if ((msg->size = VMCopy(msg->from, Write,
			   (Address) mounts,
		           (Address) msg->buffer,
			    sizeof(mounts) > msg->size ? msg->size : sizeof(mounts))))
    {
	msg->result = ESUCCESS;
    }
}

void VirtualFileSystem::newProcessHandler(FileSystemMessage *msg,
					  FileSystemMessage *reply)
{
    /* Fill in the new process. */
    procs[msg->procID].userID  = msg->userID;
    procs[msg->procID].groupID = msg->groupID;
    procs[msg->procID].files   = new Vector<FileDescriptor>;
    
    /* Success. */
    reply->result = ESUCCESS;
}

void VirtualFileSystem::killProcessHandler(FileSystemMessage *msg,
					   FileSystemMessage *reply)
{
    /* Clear process entry. */
    procs[msg->procID].userID  = 0;
    procs[msg->procID].groupID = 0;
    
    // TODO: don't forget to delete files themselves!!!
    delete procs[msg->procID].files;
    
    /* Success. */
    reply->result = ESUCCESS;
}

void VirtualFileSystem::ioDoneHandler(FileSystemMessage *msg,
				      FileSystemMessage *reply)
{
    FileDescriptor *fd;

    /* Find the file descriptor. */
    if ((fd = (*procs[msg->procID].files)[msg->fd]))
    {
	/* Increment file pointer. */
	fd->position += msg->size;

        /* Inform the user process the I/O operation has finished. */
        reply->size   = msg->size;
        reply->result = msg->result;
        reply->ipc(msg->procID, Send, sizeof(*reply));
    }
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
