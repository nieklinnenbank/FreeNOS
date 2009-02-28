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

#ifndef __FILESYSTEM_FILESYSTEMSERVER_H
#define __FILESYSTEM_FILESYSTEMSERVER_H

#include <api/IPCMessage.h>
#include <IPCServer.h>
#include <Types.h>
#include <Error.h>

/**
 * Actions which may be performed on the filesystem.
 */
typedef enum FileSystemAction
{
    CreateFile  = 0,
    OpenFile    = 1,
    ReadFile    = 2,
    WriteFile   = 3,
    CloseFile   = 4,
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
    FileSystemMessage() : action(ReadFile), buffer(ZERO), size(ZERO),
			  fd(ZERO), major(ZERO), minor(ZERO), mode(ZERO)
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
	major  = m->major;
	minor  = m->minor;
	mode   = m->mode;
    }

    union
    {
	/** Action to perform. */
	FileSystemAction action;
    
	/** Result code. */
	Error result;
    };
    
    union
    {
	/** Points to a buffer for I/O. */
	char *buffer;
    
	/** Full path to a file. */
	char *path;
    };
    
    /** Size of the buffer. */
    Size size;

    /** File descriptor. */
    u16 fd;
    
    /** Used for device files. */
    u8 major, minor;
    
    /** File permissions. */
    u16 mode;

    /** Not used. */
    u16 unused[2];
}
FileSystemMessage;

/**
 * Manages system terminals.
 */
class FileSystemServer : public IPCServer<FileSystemServer, FileSystemMessage>
{
    public:
    
	/**
	 * Class constructor function.
	 */
	FileSystemServer();
	
    private:

	/**
	 * Opens a file.
	 * @param msg Input FileSystemMessage pointer.
	 * @param reply Output FileSystemMessage pointer.
	 */
	void doOpenFile(FileSystemMessage *msg,
			FileSystemMessage *reply);
    
	/**
	 * Process a read request.
	 * @param msg Input FileSystemMessage pointer.
	 * @param reply Output FileSystemMessage pointer.
	 */
	void doReadFile(FileSystemMessage *msg,
			FileSystemMessage *reply);
	
	/**
	 * Process a write request.
	 * @param msg Input FileSystemMessage pointer.
	 * @param reply Output FileSystemMessage pointer.
	 */
	void doWriteFile(FileSystemMessage *msg,
			 FileSystemMessage *reply);
};

#endif /* __FILESYSTEM_FILESYSTEMSERVER_H */
