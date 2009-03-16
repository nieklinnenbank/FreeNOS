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
#include "File.h"
#include "Device.h"

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
	from   = m->from;
	type   = m->type;
	action = m->action;
	buffer = m->buffer;
	size   = m->size;
	fd     = m->fd;
	filetype = m->filetype;
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
	this->ipc(VFSSRV_PID, SendReceive);
    }

    union
    {
	/** Action to perform. */
	FileSystemAction action;
    
	/** Result code. */
	Error result;
    };

    /** Points to a buffer for I/O. */
    char *buffer;

    union
    {
	/** Size of the buffer. */
	Size size;
	
	/** User ID and group ID. */
	u16 userID, groupID;
    };

    union
    {
	/** File mode permissions. */
	FileMode mode;

        /** Unique identifier. */
	Address ident;
	
	/** File Statistics. */
	FileStat *stat;
    };

    union
    {
        /** File descriptor. */
	u16 fd;

	/** Process id number. */
        ProcessID procID;

	/** Filetype. */
        FileType filetype;
    };

    union
    {
	/** Offset in the file to read. */
        Size offset;
	
	/** Device major/minor numbers. */
	DeviceID deviceID;
    };
}
FileSystemMessage;

#endif /* __FILESYSTEM_FILESYSTEMSERVER_H */
