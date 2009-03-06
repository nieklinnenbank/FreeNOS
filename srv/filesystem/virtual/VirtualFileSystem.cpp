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

FileSystemMount VirtualFileSystem::mounts[MAX_MOUNTS];
UserProcessFSEntry VirtualFileSystem::procs[MAX_PROCS];

VirtualFileSystem::VirtualFileSystem()
    : IPCServer<VirtualFileSystem, FileSystemMessage>(this)
{
    SystemInformation info;
    FileSystemMessage msg;

    /* Register message handlers. */
    addIPCHandler(OpenFile,    &VirtualFileSystem::openFileHandler);
    addIPCHandler(ReadFile,    &VirtualFileSystem::readFileHandler);
    addIPCHandler(CloseFile,   &VirtualFileSystem::closeFileHandler);
    addIPCHandler(Mount,       &VirtualFileSystem::mountHandler);
    addIPCHandler(MountInfo,   &VirtualFileSystem::mountInfoHandler);
    addIPCHandler(NewProcess,  &VirtualFileSystem::newProcessHandler);
    addIPCHandler(KillProcess, &VirtualFileSystem::killProcessHandler);
    
    /* Wait for process server to sync procs. */
    for (Size i = 0; i < info.moduleCount; i++)
    {
	if (!msg.ipc(PROCSRV_PID, Receive))
	{
	    newProcessHandler(&msg, &msg);
	    msg.ipc(PROCSRV_PID, Send);
	}
    }
}

void VirtualFileSystem::openFileHandler(FileSystemMessage *msg,
					FileSystemMessage *reply)
{
    char path[PATHLEN];
    FileSystemMount *mount;
    FileSystemMessage fs;

    /* Obtain full path to open. */
    if ((reply->result = VMCopy(msg->from, Read, (Address) path,
			       (Address) msg->buffer, PATHLEN) < 0))
    {
	return;
    }
    /* Do we have a mount? */
    if (!(mount = findMount(path)))
    {
	reply->result = ENOSUCH;
	return;
    }
    // TODO: stat first, to enforce POSIX permissions here!
    
    /* Ask filesystem server to open it. */
    fs.action = OpenFile;
    fs.buffer = path + strlen(mount->path) - 1;
    IPCMessage(mount->procID, SendReceive, &fs);
    
    /* Allocate file descriptor. */
    if (!(reply->result = fs.result))
    {
	reply->fd = procs[msg->from].files->insert(new FileDescriptor(mount, fs.ident));
        procs[msg->from].fileCount++;
    }
}

void VirtualFileSystem::readFileHandler(FileSystemMessage *msg,
					FileSystemMessage *reply)
{
    FileDescriptor *fd;
    FileSystemMessage fs;
    
    /* Do we have this fd? */
    if (!(fd = (*procs[msg->from].files)[msg->fd]))
    {
	reply->result = ENOSUCH;
	return;
    }
    /* Request read operation. */
    fs.action = ReadFile;
    fs.ident  = fd->identifier;
    fs.size   = msg->size;
    fs.buffer = msg->buffer;
    fs.procID = msg->from;
    fs.offset = fd->position;
    
    /* Do IPC. */
    IPCMessage(fd->mount->procID, SendReceive, &fs);
    
    /* Success. */
    fd->position  += fs.size;
    reply->result  = fs.result;
    reply->size    = fs.size;
}

void VirtualFileSystem::closeFileHandler(FileSystemMessage *msg,
					 FileSystemMessage *reply)
{
    FileSystemMessage fs;
    FileDescriptor *fd;

    /* Do we have it opened? */
    if ((fd = (*procs[msg->from].files)[msg->fd]))
    {
	/* Inform filesystem we are closing the file. */
	fs.action = CloseFile;
	fs.fd     = msg->fd;
	
	/* Send message. */
	IPCMessage(fd->mount->procID, SendReceive, &fs);
	reply->result = fs.result;
	
	/* Release file. */
	procs[msg->from].files->remove(msg->fd);
	procs[msg->from].fileCount--;
	// TODO: crash??? delete fd;
    }
    else
	reply->result = ENOSUCH;
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
    delete procs[msg->procID].files;
    
    /* Success. */
    reply->result = ESUCCESS;
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
	Size len = strlen(mounts[i].path);
    
	if (strncmp(path, mounts[i].path, len) == 0)
	{
	    return &mounts[i];
	}
    }
    return (FileSystemMount *) ZERO;
}
