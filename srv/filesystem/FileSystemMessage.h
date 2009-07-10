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

#ifndef __FILESYSTEM_FILESYSTEMMESSAGE_H
#define __FILESYSTEM_FILESYSTEMMESSAGE_H

#include <API/IPCMessage.h>
#include <IPCServer.h>
#include <Types.h>
#include <Error.h>
#include "FileType.h"
#include "FileMode.h"
#include "FileStat.h"

/**
 * Actions which may be performed on the filesystem.
 */
typedef enum FileSystemAction
{
    CreateFile    = 0,
    OpenFile      = 1,
    ReadFile      = 2,
    WriteFile     = 3,
    SeekFile      = 4,
    StatFile      = 5,
    ChangeFile    = 6,
    CloseFile     = 7,
    Mount	  = 8,
    Unmount  	  = 9,
    MountInfo     = 10,
    NewProcess    = 11,
    KillProcess   = 12,
    IODone        = 13,
    GetCurrentDir = 14,
    SetCurrentDir = 15,
}
FileSystemAction;

/**
 * FileSystem IPC message.
 */
typedef struct FileSystemMessage : public Message
{
    /**
     * Default constructor.
     */
    FileSystemMessage() : action(ReadFile)
    {
    }

    /**
     * Assignment operator.
     * @param m FileSystemMessage pointer to copy from.
     */
    void operator = (FileSystemMessage *m)
    {
	from        = m->from;
	type        = m->type;
	action      = m->action;
	savedAction = m->savedAction;
	result      = m->result;
	buffer      = m->buffer;
	size        = m->size;
	offset      = m->offset;
	userID      = m->userID;
	groupID     = m->groupID;
	deviceID    = m->deviceID;
	mode        = m->mode;
	stat        = m->stat;
	fd          = m->fd;
	filetype    = m->filetype;
	ident       = m->ident;
	procID      = m->procID;
    }

    /**
     * Introduce a new process to VFS.
     * @param pid Process ID number.
     * @param ppid Parent Process ID number.
     * @param uid User ID number.
     * @param gid Group ID number.
     */
    void newProcess(ProcessID pid, ProcessID ppid, u16 uid, u16 gid)
    {
	this->procID   = pid;
	this->parentID = ppid;
	this->userID   = uid;
	this->groupID  = gid;
	this->action   = NewProcess;
	this->ipc(VFSSRV_PID, SendReceive, sizeof(*this));
    }

    /**
     * Attempt to create a new file.
     * @param path Path to the file to create.
     * @param type File type.
     * @param mode Initial file permissions.
     * @param major Device major ID (optional).
     * @param minor Device minor ID (optional).
     * @param pid Process to send the request to.
     */
    void createFile(char *path, FileType type = RegularFile,
		    FileMode mode = OwnerRW, u16 major = ZERO,
		    u16 minor = ZERO, ProcessID pid = VFSSRV_PID)
    {
	this->action   = CreateFile;
	this->buffer   = path;
	this->filetype = type;
	this->mode     = mode;
	this->deviceID.major = major;
	this->deviceID.minor = minor;
	this->ipc(pid, SendReceive, sizeof(*this));
    }
    
    /**
     * Open a file on the filesystem.
     * @param path Path to the file to open.
     * @param pid Process ID number of the filesystem.
     */
    Error openFile(char *path, ProcessID pid = VFSSRV_PID)
    {
	action = OpenFile;
	buffer = path;
	ipc(pid, SendReceive, sizeof(*this));
	return result;
    }
    
    /**
     * Read a file from the filesystem.
     * @param buf Output buffer.
     * @param sz Maximum size to read.
     * @param off Offset in the file to read.
     * @param pid ProcessID number of the filesystem.
     * @return Number of bytes read on success and Error code on failure.
     */
    Error readFile(char *buf, Size sz, Size off = ZERO,
		   ProcessID pid = VFSSRV_PID)
    {
	action = ReadFile;
	buffer = buf;
	size   = sz;
	offset = off;
	ipc(pid, SendReceive, sizeof(*this));
	return result == ESUCCESS ? size : result;
    }

    /**
     * Write to a file on the filesystem.
     * @param buf Input buffer.
     * @param sz Maximum size to write.
     * @param off Offset in the file to write.
     * @param pid ProcessID number of the filesystem.
     * @return Number of bytes written on success and Error code on failure.
     */
    Error writeFile(char *buf, Size sz, Size off = ZERO,
		   ProcessID pid = VFSSRV_PID)
    {
	action = WriteFile;
	buffer = buf;
	size   = sz;
	offset = off;
	ipc(pid, SendReceive, sizeof(*this));
	return result == ESUCCESS ? size : result;
    }

    /**
     * Moves the filepointer of a file.
     * @param fildes File descriptor
     * @param off Offset to move to.
     * @param how Determines how to move the file descriptor.
     * @return Error code of the seek operation.
     */
    Error seekFile(u16 fildes, Size off, int how, ProcessID pid = VFSSRV_PID)
    {
	action = SeekFile;
	fd     = fildes;
	offset = off;
	// TODO: use the how argument!
	// size = how;
	ipc(pid, SendReceive, sizeof(*this));
	return result;
    }

    /**
     * Closes an open file.
     * @param fd File descriptor number.
     * @param pid Process ID number of the filesystem.
     * @return Error code of the closing operation.
     */
    Error closeFile(int fd, ProcessID pid = VFSSRV_PID)
    {
	action   = CloseFile;
	this->fd = fd;
	ipc(pid, SendReceive, sizeof(*this));
	return result;
    }

    /**
     * I/O operation has completed.
     * @param fs Process of the filesystem.
     * @param pid Process ID for which I/O has been performed.
     * @param bytes Number of bytes processed.
     * @param e Result code.
     */
    void ioDone(ProcessID fs, ProcessID pid, Size bytes, Error e)
    {
	procID = pid;
	size   = bytes;
	result = e;
	action = IODone;
	ipc(fs, Send, sizeof(*this));
    }

    /**
     * Send an error back to the transient Process.
     * @param result Error code.
     * @param pid ProcessID to send to.
     * @param ac Action override.
     */
    void error(Error err, FileSystemAction ac = IODone, ProcessID pid = ANY)
    {
	result = err;
	action = ac;
	ipc(pid == ANY ? from : pid, Send, sizeof(*this));
    }

    /** Action to perform. */
    FileSystemAction action, savedAction;
    
    /** Result code. */
    Error result;

    /** Points to a buffer for I/O. */
    char *buffer;

    /** Size of the buffer. */
    Size size;

    /** Offset in the file to read. */
    Size offset;
	
    /** User ID and group ID. */
    u16 userID, groupID;

    /** Filetype. */
    FileType filetype;

    /** File mode permissions. */
    FileMode mode;
        
    /** File Statistics. */
    FileStat *stat;

    /** File descriptor. */
    u16 fd;
    
    /** Unique identifier. */
    Address ident;

    /** Process id number. */
    ProcessID procID;

    union
    {	
	/** Device major/minor numbers. */
	DeviceID deviceID;
	
	/** Parent process ID number. */
	ProcessID parentID;
    };
}
FileSystemMessage;

#endif /* __FILESYSTEM_FILESYSTEMSERVER_H */
