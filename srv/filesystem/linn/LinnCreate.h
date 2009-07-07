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

#ifndef __FILESYSTEM_LINN_CREATE_H
#define __FILESYSTEM_LINN_CREATE_H

#include <BitMap.h>
#include <List.h>
#include <String.h>
#include "LinnSuperBlock.h"
#include "LinnInode.h"

/** Default block size. */
#define LINN_CREATE_BLOCK_SIZE		2048

/** Default number of blocks to allocate. */
#define LINN_CREATE_BLOCK_NUM		8192

/** Default number of data blocks per group descriptor. */
#define LINN_CREATE_BLOCKS_PER_GROUP	8192

/** Default number of inodes to allocate. */
#define LINN_CREATE_INODE_NUM		1024

/** Default number of inodes per group descriptor. */
#define LINN_CREATE_INODES_PER_GROUP	1024

/**
 * @brief Returns a pointer to the correct in-memory block.
 * @param type Data type to return a pointer for.
 * @param nr Block number.
 * @return A pointer of the given type.
 */
#define BLOCKPTR(type,nr) (type *)(blocks + (super->blockSize * (nr))) 

/**
 * Retrieve a given number of free contigeous blocks.
 * @param sb LinnSuperBlock pointer.
 * @return Block number of the first block in the contigeous array of blocks.
 */
#define BLOCKS(sb,count) \
    ({ \
        if ((sb)->freeBlocksCount < (count)) \
	{ \
	    printf("%s: not enough free blocks remaining (%llu needed)\n", \
		    prog, (count)); \
	    exit(EXIT_FAILURE); \
	} \
	(sb)->freeBlocksCount -= (count); \
	((sb)->blocksCount - (sb)->freeBlocksCount - (count)); \
     }) 

/**
 * Retrieve one free block.
 * @param sb LinnSuperBlock pointer.
 * @return Block number of a free block.
 */
#define BLOCK(sb) \
    BLOCKS(sb, ((u64)1))

/**
 * Class for creating new Linnenbank FileSystems.
 */
class LinnCreate
{
    public:
    
	/**
	 * Class constructor.
	 */
	LinnCreate();

	/**
	 * Creates the LinnFS FileSystem.
	 * @param blockSize The size of each block in the new filesystem.
	 * @param blockNum The maximum number of blocks in the new filesystem.
	 * @param inodeNum Number of inodes to allocate.
	 * @return EXIT_SUCCESS if successfull and EXIT_FAILURE otherwise.
	 */
	int create(Size blockSize, Size blockNum, Size inodeNum);

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
	 * Creates an empty LinnInode.
	 * @param inodeNum Inode number.
	 * @param type Type of file.
	 * @param mode Access permissions.
	 * @param uid User identity.
	 * @param gid Group identity.
	 * @return LinnInode pointer.
	 */
	LinnInode * createInode(le64 inodeNum, FileType type, FileModes mode,
                    		UserID uid = ZERO, GroupID gid = ZERO);

	/**
	 * Copies a local file contents into an LinnInode.
	 * @param inputFile Path to the local file to insert.
	 * @param st POSIX stat pointer for the local file.
	 * @return Inode number of the inserted file.
	 */
	le64 createInode(char *inputFile, struct stat *st);

	/**
	 * Inserts the given directory and it's childs to the filesystem image.
	 * @param inputFile Path to a local directory.
	 * @param inodeNum Inode number for the input directory.
	 * @param parentNum Inode number of our parent.
	 * @note This function is recursive.
	 */
	void insertDirectory(char *inputFile, le64 inodeNum, le64 parentNum);

	void insertFile(char *inputFile, LinnInode *inode,
                        struct stat *st);

	/**
	 * Writes the final image to disk.
	 * @return EXIT_SUCCESS if successfull and EXIT_FAILURE otherwise.
	 */
	int writeImage();
	
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
	LinnSuperBlock *super;
	
	/** Block and Inode bitmaps. */
	BitMap *blockMap, *inodeMap;

	/** Array of blocks available in the filesystem. */
	u8 *blocks;
};

#endif /* __FILESYSTEM_LINN_CREATE_H */
