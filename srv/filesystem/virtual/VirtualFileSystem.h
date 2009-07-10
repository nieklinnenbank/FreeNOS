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

#ifndef __FILESYSTEM_VIRTUALFILESYSTEM_H
#define __FILESYSTEM_VIRTUALFILESYSTEM_H

#include <API/VMCopy.h>
#include <API/SystemInfo.h>
#include <FreeNOS/Process.h>
#include <IPCServer.h>
#include <FileSystem.h>
#include <FileSystemMessage.h>
#include <Array.h>

/** 
 * @defgroup vfs vfs (Virtual Filesystem) 
 * @{ 
 */

/** Maximum number of mounted filesystems. */
#define MAX_MOUNTS 16

/**
 * Represents a mounted filesystem.
 */
typedef struct FileSystemMount
{
    /** Path of the mount. */
    char path[PATHLEN];
    
    /** Server which is responsible for the mount. */
    ProcessID procID;
    
    /** Mount options. */
    ulong options;
}
FileSystemMount;

/**
 * Abstracts a file which is opened by a user process.
 */
typedef struct FileDescriptor
{
    /**
     * Constructor function.
     * @param mount Corresponding mount.
     * @param ident Unique identifier.
     */
    FileDescriptor(FileSystemMount *m, Address ident)
	: mount(m), identifier(ident), position(ZERO)
    {
    }

    /** Mount on which this file was opened. */
    FileSystemMount *mount;

    /** Unique identifier, used by the underlying filesystem. */
    Address identifier;

    /** Current position indicator. */
    Size position;
}
FileDescriptor;

/**
 * Filesystem's process table.
 */
typedef struct UserProcessFSEntry
{
    /** User and group ID. */
    u16 userID, groupID;

    /** Open files. */
    Array<FileDescriptor> *files;
    
    /** Total number of open files. */
    Size fileCount;
    
    /** Current working directory. */
    char currentDir[PATHLEN];
}
UserProcessFSEntry;

/**
 * Represents filesystems via an uniform interface to user applications.
 */
class VirtualFileSystem : public IPCServer<VirtualFileSystem, FileSystemMessage>
{
    public:
    
	/**
	 * Constructor function.
	 */
	VirtualFileSystem();

    private:

	/**
	 * Performs filesystem Input/Output.
	 * @param msg Input message.
	 */
	void ioHandler(FileSystemMessage *msg);

	/**
	 * Informs us that a I/O operation has completed.
	 * @param msg Input message.
	 */
	void ioDoneHandler(FileSystemMessage *msg);

	/**
	 * Mounts a filesystem.
	 * @param msg Input message.
	 */
	void mountHandler(FileSystemMessage *msg);

	/**
	 * Request mounted filesystems.
	 * @param msg Input message.
	 */
	void mountInfoHandler(FileSystemMessage *msg);

	/**
	 * Allows the process server to communicate process+uid/gid pairs.
	 * @param msg Input message.
	 */
	void newProcessHandler(FileSystemMessage *msg);
	
	/**
	 * Allows the process server to remove process+uid/gid pairs.
	 * @param msg Input message.
	 */
	void killProcessHandler(FileSystemMessage *msg);

	/**
	 * Get or set the current directory.
	 * @param msg Input message.
	 */
	void currentDirHandler(FileSystemMessage *msg);

	/**
	 * Creats a new mount.
	 * @param path Full path of the new mount.
	 * @param pid Process which handles the mountpoint.
	 * @param opts Mount options.
	 */
	void insertMount(char *path, ProcessID pid, ulong opts);

	/**
	 * Lookup the mount for a given path.
	 * @param msg Message containing a path pointer.
	 * @return Pointer to mount if found, ZERO otherwise.
	 */
	FileSystemMount * findMount(FileSystemMessage *msg);
	
	/** Mounted filesystems. */
	static FileSystemMount mounts[MAX_MOUNTS];
	
	/** Filesystem's user process table. */
	static UserProcessFSEntry procs[MAX_PROCS];
};

/**
 * @}
 */

#endif /* __FILESYSTEM_VIRTUALFILESYSTEM_H */
