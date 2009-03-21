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

#include <arch/Process.h>
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
	    : type(t), size(ZERO), uid(u), gid(g)
	{
	}

	/**
	 * Destructor function.
	 */
	virtual ~File()
	{
	}
    
	/**
	 * Read bytes from the file.
	 * @param msg Describes the read request.
	 * @return Number of bytes read on success, Error on failure.
	 */
	virtual Error read(FileSystemMessage *msg)
	{
	    return ENOSUPPORT;
	}

	/**
	 * Write bytes to the file.
	 * @param msg Describes the write request.
	 * @return Number of bytes written on success, Error on failure.
	 */
	virtual Error write(FileSystemMessage *msg)
	{
	    return ENOSUPPORT;
	}
    
	/**
	 * Retrieve file statistics.
	 * @param st Buffer to write statistics to.
	 */
	virtual void status(struct stat *st)
	{
	    st->st_mode = type;
	    st->st_size = size;
	    st->st_uid  = uid;
	    st->st_gid  = gid;
	}
	
	/**
	 * Retrieve our filetype.
	 * @return FileType object.
	 */
	FileType getType()
	{
	    return type;
	}
    
    protected:

	/** File of this file. */
	FileType type;
	
	/** Size of the file, in bytes. */
	Size size;
	
	/** Owner of the file. */
	UserID uid;
	
	/** Group of the file. */
	GroupID gid;
};

#endif /* __FILESYSTEM_FILE_H */
