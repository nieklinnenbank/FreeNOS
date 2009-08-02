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
#include "FileSystemMessage.h"
#include "FileType.h"
#include "FileMode.h"

/**
 * @brief Abstracts a file present on a FileSystem.
 * @see FileSystem
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
	    : type(t), access(OwnerRWX), size(ZERO),
	      openCount(ZERO), uid(u), gid(g)
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
	 * Get the number of times we are opened by a process.
	 * @return Open count.
	 * @see openCount
	 */
	Size getOpenCount()
	{
	    return openCount;
	}

	/**
	 * Attempt to open a file.
	 * @param msg Describes the open request.
	 * @param pid Process Identity to serve us from. May be changed
	 *            to redirect to other servers.
	 * @param ident Identity to be filled in the FileDescriptor.
	 * @return Error code status.
	 */
	virtual Error open(FileSystemMessage *msg,
			   ProcessID *pid, Address *ident)
	{
	    openCount++;
	    return ESUCCESS;
	}

	/**
	 * @brief Attempt to close a file.
	 * @param msg Describes the closing request.
	 * @return Error code status.
	 */
	virtual Error close(FileSystemMessage *msg)
	{
	    openCount--;
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
	    FileStat st;
	    Error e;
	
	    /* Fill in the status structure. */
	    st.type     = type;
	    st.access   = access;
	    st.size     = size;
	    st.userID   = uid;
	    st.groupID  = gid;
	    
	    /* Copy to the remote process. */
	    if ((e = VMCopy(msg->from, Write, (Address) &st,
			   (Address) msg->stat, sizeof(st)) > 0))
	    {
		return ESUCCESS;
	    }
	    else
		return e;
	}
    
    protected:

	/** Type of this file. */
	FileType type;
	
	/** Access permissions. */
	FileModes access;
	
	/** Size of the file, in bytes. */
	Size size;
	
	/** Number of times the File has been opened by a process. */
	Size openCount;
	
	/** Owner of the file. */
	UserID uid;
	
	/** Group of the file. */
	GroupID gid;
};

#endif /* __FILESYSTEM_FILE_H */
