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

#ifndef __FILESYSTEM_EXT2CREATE_H
#define __FILESYSTEM_EXT2CREATE_H

#include <BitMap.h>
#include <List.h>
#include <String.h>
#include "Ext2SuperBlock.h"
#include "Ext2Inode.h"
#include "Ext2Group.h"
#include "Ext2FileSystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/**
 * Class for creating new Extended 2 FileSystems.
 */
class Ext2Create
{
    public:
    
	/**
	 * Class constructor.
	 */
	Ext2Create();

	/**
	 * Creates a new Extended 2 FileSystem.
	 * @return EXIT_SUCCESS if successfull and EXIT_FAILURE otherwise.
	 */
	int create();

	/**
	 * Set the program name we are invoked with.
	 * @param progName program name.
	 */
	void setProgram(char *progName);
	
	/**
	 * Set the output image file name.
	 * @param imageName Image name to use.
	 */
	void setImage(char *imageName);

    private:
    
	/**
	 * Allocate and initialize a superblock.
	 */
	Ext2SuperBlock * initSuperBlock();
	
	/** Program name we are invoked with. */
	char *prog;
	
	/** Path to the output image. */
	char *image;
	
	/** Pointer to the superblock. */
	Ext2SuperBlock *super;
	
	/** List of file patterns to ignore. */
	List<String> excludes;
	
	/** Size of each block. */
        Size blockSize;
	
	/** The total number of Inode available. */
        Size totalInodes;
	
	/** Number of free Inodes. */
	Size freeInodes;
	
	/** The total number of blocks available. */
	Size totalBlocks;
	
	/** Number of free blocks. */
	Size freeBlocks;
	
	/** Size of a fragment. */
	Size fragmentSize;
};

#endif /* __FILESYSTEM_EXT2CREATE_H */
