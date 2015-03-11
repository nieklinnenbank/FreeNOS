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

#ifndef __FILESYSTEM_EXT2FILE_H
#define __FILESYSTEM_EXT2FILE_H

#include <File.h>
#include <FileSystemMessage.h>
#include <Types.h>
#include <Error.h>
#include "Ext2FileSystem.h"
#include "Ext2Inode.h"
#include "IOBuffer.h"

/**
 * @defgroup ext2 ext2fs (Extended 2 Filesystem)
 * @{
 */

/**
 * Second Extended FileSystem (ext2) file.
 */
class Ext2File : public File
{
    public:

	/**
	 * Constructor function.
	 * @param fs Extended 2 filesystem pointer.
	 * @param inode Inode pointer.
	 */
	Ext2File(Ext2FileSystem *fs, Ext2Inode *inode);

	/**
	 * Destructor function.
	 */
	~Ext2File();

	/**
	 * @brief Read out the file.
         * @param buffer Input/Output buffer to output bytes to. 
         * @param size Number of bytes to read, at maximum. 
         * @param offset Offset inside the file to start reading. 
         * @return Number of bytes read on success, Error on failure. 
	 */
	Error read(IOBuffer *buffer, Size size, Size offset);

    private:

	/** Filesystem pointer. */
	Ext2FileSystem *ext2;
	
	/** Inode pointer. */
	Ext2Inode *inode;
};

/**
 * @}
 */

#endif /* __FILESYSTEM_EXT2FILE_H */
