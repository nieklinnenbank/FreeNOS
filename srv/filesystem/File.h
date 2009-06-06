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

#ifndef __FILESYSTEM_FILE_H
#define __FILESYSTEM_FILE_H

#include <API/VMCopy.h>
#include <FreeNOS/Process.h>
#include <Types.h>
#include <Error.h>
#include <sys/stat.h>
#include "FileSystemMessage.h"

/**
 * Abstracts a file which is opened by a process.
 */
class File
{
    public:
    
	/**
	 * Constructor function.
	 * @param t Type of file.
	 * @param u User identity.
	 * @param g Group identity.
	 */
	File(FileType t = RegularFile, UserID u = ZERO, GroupID g = ZERO)
	    : type(t), size(ZERO), refCount(ZERO), openCount(ZERO),
	      uid(u), gid(g)
	{
	}

	/**
	 * Destructor function.
	 */
	virtual ~File()
	{
	}

	/**
	 * Retrieve our filetype.
	 * @return FileType object.
	 */
	FileType getType()
	{
	    return type;
	}

	/**
	 * Get the number of times we are referenced internally.
	 * @return Reference count.
	 * @see refCount
	 */
	Size getRefCount()
	{
	    return refCount;
	}
	
	/**
	 * Enlarge the number of times we are referenced by one.
	 * @see refCount
	 */
	void incrementRefCount()
	{
	    refCount++;
	}

	/**
	 * Shrink the number of times we are referenced by one.
	 * @see refCount
	 */
	void decrementRefCount()
	{
	    if (refCount > 0) refCount--;
	}


	/**
	 * Get the number of times we are opened by a process.
	 * @return Open count.
	 * @see openCount
	 */
	Size getOpenCount()
	{
	    return openCount;
	}
	
	/**
	 * Enlarge the number of times we are opened by one.
	 * @see openCount
	 */
	void incrementOpenCount()
	{
	    openCount++;
	}

	/**
	 * Shrink the number of times we are opened by one.
	 * @see openCount
	 */
	void decrementOpenCount()
	{
	    if (openCount > 0) openCount--;
	}

	/**
	 * Attempt to open a file.
	 * @param msg Describes the open request.
	 * @return Error code status.
	 */
	virtual Error open(FileSystemMessage *msg)
	{
	    return ESUCCESS;
	}
    
	/**
	 * Read bytes from the file.
	 * @param msg Describes the read request.
	 * @return Number of bytes read on success, Error on failure.
	 */
	virtual Error read(FileSystemMessage *msg)
	{
	    return ENOTSUP;
	}

	/**
	 * Write bytes to the file.
	 * @param msg Describes the write request.
	 * @return Number of bytes written on success, Error on failure.
	 */
	virtual Error write(FileSystemMessage *msg)
	{
	    return ENOTSUP;
	}
    
	/**
	 * Retrieve file statistics.
	 * @param st Buffer to write statistics to.
	 */
	virtual Error status(FileSystemMessage *msg)
	{
	    struct stat st;
	    Error e;
	
	    /* Fill in the status structure. */
	    st.st_mode = type;
	    st.st_size = size;
	    st.st_uid  = uid;
	    st.st_gid  = gid;
	    
	    /* Copy to the remote process. */
	    if ((e = VMCopy(msg->procID, Write, (Address) &st,
			   (Address) msg->stat, sizeof(st)) > 0))
	    {
		return ESUCCESS;
	    }
	    else
		return e;
	}
    
    protected:

	/** File of this file. */
	FileType type;
	
	/** Size of the file, in bytes. */
	Size size;
	
	/** Number of times the File is referenced internally. */
	Size refCount;
	
	/** Number of times the File has been opened by a process. */
	Size openCount;
	
	/** Owner of the file. */
	UserID uid;
	
	/** Group of the file. */
	GroupID gid;
};

#endif /* __FILESYSTEM_FILE_H */
