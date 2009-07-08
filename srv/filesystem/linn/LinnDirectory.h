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

#ifndef __FILESYSTEM_LINN_DIRECTORY_H
#define __FILESYSTEM_LINN_DIRECTORY_H
#ifndef __HOST__

#include <FileSystemMessage.h>
#include <Directory.h>
#include <Types.h>
#include <Error.h>
#include "LinnDirectoryEntry.h"
#include "LinnFileSystem.h"
#include "LinnInode.h"

/**
 * @defgroup linn Linnenbank Filesystem (LinnFS)
 * @{
 */

/**
 * Represents an directory on a LinnFS filesystem.
 * @see Directory
 * @see LinnDirectoryEntry
 * @see LinnFileSystem
 */
class LinnDirectory : public Directory
{
    public:
    
	/**
	 * Constructor function.
	 * @param fs Filesystem pointer.
	 * @param inode Inode pointer.
	 * @see LinnFileSystem
	 * @see LinnInode
	 */
	LinnDirectory(LinnFileSystem *fs, LinnInode *inode);

        /** 
         * Read directory entries. 
         * @param msg Read request. 
         * @return Number of bytes read on success, Error on failure. 
         */
	Error read(FileSystemMessage *msg);

	/**
	 * Retrieve a directory entry.
	 * @param dent LinnDirectoryEntry buffer pointer.
	 * @param name Unique name of the entry.
	 * @return ESUCCESS if found, or an error code otherwise.
	 */
	Error getEntry(LinnDirectoryEntry *dent, char *name);

    private:

	/** Filesystem pointer. */
	LinnFileSystem *fs;

	/** Inode which describes the directory. */
	LinnInode *inode;
};

/**
 * @}
 */

#endif /* __HOST__ */
#endif /* __FILESYSTEM_EXT2DIRECTORY_H */
