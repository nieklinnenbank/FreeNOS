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

#include <api/VMCopy.h>
#include <api/SystemInfo.h>
#include <arch/Process.h>
#include <IPCServer.h>
#include <Vector.h>
#include <FileSystem.h>
#include <FileSystemMessage.h>

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
    Vector<FileDescriptor> *files;
    
    /** Total number of open files. */
    Size fileCount;
    
    /** Root directory
     char rootdir[len]
      */
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
	 * Creates a new file.
	 * @param msg Input message.
	 * @param reply Response message.
	 */
	void createFileHandler(FileSystemMessage *msg,
			       FileSystemMessage *reply);

	/**
	 * Opens a new file.
	 * @param msg Input message.
	 * @param reply Response message.
	 */
	void openFileHandler(FileSystemMessage *msg,
			     FileSystemMessage *reply);

	/**
	 * Reads a file.
	 * @param msg Input message.
	 * @param reply Response message.
	 */
	void readFileHandler(FileSystemMessage *msg,
			     FileSystemMessage *reply);

	/**
	 * Closes a file.
	 * @param msg Input message.
	 * @param reply Response message.
	 */
	void closeFileHandler(FileSystemMessage *msg,
			      FileSystemMessage *reply);

	/**
	 * Get file statistics.
	 * @param msg Input message.
	 * @param reply Response message.
	 */
	void statFileHandler(FileSystemMessage *msg,
			     FileSystemMessage *reply);

	/**
	 * Mounts a filesystem.
	 * @param msg Input message.
	 * @param reply Response message.
	 */
	void mountHandler(FileSystemMessage *msg,
		          FileSystemMessage *reply);

	/**
	 * Request mounted filesystems.
	 * @param msg Input message.
	 * @param reply Response message.
	 */
	void mountInfoHandler(FileSystemMessage *msg,
			      FileSystemMessage *reply);

	/**
	 * Allows the process server to communicate process+uid/gid pairs.
	 * @param msg Input message.
	 * @param reply Response message.
	 */
	void newProcessHandler(FileSystemMessage *msg,
			       FileSystemMessage *reply);
	
	/**
	 * Allows the process server to remove process+uid/gid pairs.
	 * @param msg Input message.
	 * @param reply Response message.
	 */
	void killProcessHandler(FileSystemMessage *msg,
			        FileSystemMessage *reply);

	/**
	 * Informs us that a I/O operation has completed.
	 * @param msg Input message.
	 * @param reply Response message.
	 */
	void ioDoneHandler(FileSystemMessage *msg,
			   FileSystemMessage *reply);
	
	/**
	 * Creats a new mount.
	 * @param path Full path of the new mount.
	 * @param pid Process which handles the mountpoint.
	 * @param opts Mount options.
	 */
	void insertMount(char *path, ProcessID pid, ulong opts);

	/**
	 * Lookup the mount for a given path.
	 * @param path Path of the mountpoint to find.
	 * @return Pointer to mount if found, ZERO otherwise.
	 */
	FileSystemMount * findMount(char *path);
	
	/** Mounted filesystems. */
	static FileSystemMount mounts[MAX_MOUNTS];
	
	/** Filesystem's user process table. */
	static UserProcessFSEntry procs[MAX_PROCS];
};

#endif /* __FILESYSTEM_VIRTUALFILESYSTEM_H */
