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

#include <api/IPCMessage.h>
#include <IPCServer.h>
#include <Types.h>
#include <Error.h>
#include <sys/stat.h>

/**
 * Actions which may be performed on the filesystem.
 */
typedef enum FileSystemAction
{
    CreateFile  = 0,
    OpenFile    = 1,
    ReadFile    = 2,
    WriteFile   = 3,
    StatFile    = 4,
    ChangeFile  = 5,
    CloseFile   = 6,
    Mount	= 7,
    Unmount	= 8,
    MountInfo   = 9,
    NewProcess  = 10,
    KillProcess = 11,
    IODone      = 12,
}
FileSystemAction;

/** 
 * All possible filetypes. 
 */
typedef enum FileType
{
    RegularFile         = S_IFREG,
    DirectoryFile       = S_IFDIR,
    BlockDeviceFile     = S_IFBLK,
    CharacterDeviceFile = S_IFCHR,
    SymlinkFile         = S_IFLNK,
    FIFOFile            = S_IFIFO,
}
FileType;

/** File access permissions. */
typedef uint FileMode;

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
	from   = m->from;
	type   = m->type;
	action = m->action;
	result = m->result;
	buffer = m->buffer;
	size   = m->size;
	offset = m->offset;
	userID = m->userID;
	groupID = m->groupID;
	deviceID = m->deviceID;
	mode   = m->mode;
	stat   = m->stat;
	fd     = m->fd;
	filetype = m->filetype;
	ident    = m->ident;
	procID   = m->procID;
    }

    /**
     * Introduce a new process to VFS.
     * @param pid Process ID number.
     * @param uid User ID number.
     * @param gid Group ID number.
     */
    void newProcess(ProcessID pid, u16 uid, u16 gid)
    {
	this->procID  = pid;
	this->userID  = uid;
	this->groupID = gid;
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
		    FileMode mode = 0600, u16 major = ZERO,
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
    Error readFile(char *buf, Size sz, Size off,
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

    /** Action to perform. */
    FileSystemAction action;
    
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
	
    /** Device major/minor numbers. */
    DeviceID deviceID;
}
FileSystemMessage;

#endif /* __FILESYSTEM_FILESYSTEMSERVER_H */
