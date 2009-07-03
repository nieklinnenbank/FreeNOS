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
#include <sys/types.h>

/** Default number of data blocks per group descriptor. */
#define EXT2CREATE_BLOCKS_PER_GROUP	8192

/** Default number of fragments per group descriptor. */
#define EXT2CREATE_FRAGS_PER_GROUP	8192

/** Default number of inodes per group descriptor. */
#define EXT2CREATE_INODES_PER_GROUP	1024

/** Maximum number of blocks we support. */
#define EXT2CREATE_NUM_BLOCKS		8192

/**
 * @brief Macro used in Ext2Create::readInput() to create directory entries.
 * @param f File name.
 * @param i Inode number.
 */
#define EXT2_CREATE_DIRENT(f,i) \
    if (dprev) \
    { \
	dprev->recordLength  = (dprev->nameLength + 8); \
	dprev->recordLength += 4 - (dprev->recordLength % 4); \
    } \
    dprev = dent; \
    dent->inode        = (i); \
    dent->nameLength   = strlen(f); \
    dent->recordLength = last - ((Address) dent); \
    strncpy((dent)->name, (f), EXT2_NAME_LEN); \
    \
    {ulong ptr = (ulong) dent; \
    ptr += dent->nameLength + 8; \
    ptr += 4 - (ptr % 4); \
    dent = (Ext2DirectoryEntry *) ptr;}

/**
 * @brief Returns a pointer to the correct in-memory block.
 * @param type Data type to return a pointer for.
 * @param nr Block number.
 * @return A pointer of the given type.
 */
#define BLOCKPTR(type,nr) (type *)(blocks + (blockSize * (nr))) 

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
	 * Traverse the input directory.
	 * @param directory Path of the local directory to traverse.
	 * @param parent Parent Inode number.
	 * @return Inode number for the created directory.
	 */
	Size readInput(char *directory, Size parent);

	/**
	 * Writes the final image to disk.
	 * @return EXIT_SUCCESS if successfull and EXIT_FAILURE otherwise.
	 */
	int writeImage();

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

	/**
	 * Set the input directory name.
	 * @param inputName Input directory to use.
	 */
	void setInput(char *inputName);

	/**
	 * Exclude files matching the given pattern from the image.
	 * @param pattern Pattern to match against.
	 */
	void setExclude(char *pattern);

	/**
	 * Output verbose messages during the construction.
	 * @param newVerbose True to turn on, false to turn off verbose messages.
	 */
	void setVerbose(bool newVerbose);

    private:

	/**
	 * Initialize a superblock.
	 * @param ptr Superblock buffer.
	 */
	void initSuperBlock(Ext2SuperBlock *ptr);
	
	/**
	 * Initialize a group descriptor.
	 * @param grp Group pointer.
	 */
	void initGroup(Ext2Group *grp);

	/**
	 * Allocates a new Extended 2 Inode for the given file.
	 * @param inputFile Path to the local file.
	 * @param Pointer to fill in the inode number.
	 * @return Pointer to the Ext2Inode.
	 */	
	Ext2Inode * createInode(char *inputFile, Size *number);
	
	/** Program name we are invoked with. */
	char *prog;
	
	/** Path to the output image. */
	char *image;
	
	/** Path to the input directory. */
	char *input;

	/** Output verbose messages. */
	bool verbose;

	/** List of file patterns to ignore. */
	List<String> excludes;
	
	/** Pointer to the superblock. */
	Ext2SuperBlock *super;
	
	/** Pointer to the group descriptor. */
	Ext2Group *group;
	
	/** Block and Inode bitmaps. */
	BitMap *blockMap, *inodeMap;

	/** Array of blocks available in the filesystem. */
	u8 *blocks;
		
	/** Size of each block. */
        Size blockSize;
};

#endif /* __FILESYSTEM_EXT2CREATE_H */
