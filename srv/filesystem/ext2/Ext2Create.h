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

/**
 * Describes an input file to be placed on the new filesystem.
 */
typedef struct Ext2InputFile
{
    /** Name of the file. */
    char name[EXT2_NAME_LEN];

    /** Inode of the file. */
    Ext2Inode inode;
    
    /** Inode number. */
    le32 inodeNumber;
    
    /** List of childs. */
    List<Ext2InputFile> childs;
}
Ext2InputFile;

typedef struct Ext2CreateGroup
{
    /** Group descriptor. */
    Ext2Group group;
    
    /** In-memory block bitmap. */
    BitMap *blockMap;
    
    /** In-memory inode bitmap. */
    BitMap *inodeMap;
    
    /** In-memory inode table. */
    Ext2Inode *inodes;
}
Ext2CreateGroup;

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
	 * @param parent Parent directory or ZERO if none.
	 * @return EXIT_SUCCESS if successfull and EXIT_FAILURE otherwise.
	 */
	int readInput(char *directory, Ext2InputFile *parent);

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

    private:

	/**
	 * Adds the given input file to it's parent.
	 * @param inputFile Path of the local file to add.
	 * @param parent Pointer to an Ext2InputFile.
	 * @return Pointer to the newly created Ext2InputFile.
	 */
	Ext2InputFile * addInputFile(char *inputFile, Ext2InputFile *parent);

	/**
	 * Create group descriptors and add the given input files.
	 * @param list List to store group descriptors in.
	 * @param file Input file and childs to add.
	 */
	void createGroups(List<Ext2CreateGroup> *list, Ext2InputFile *file);
	
	/**
	 * Add the given input file to any group.
	 * @param list List of available group descriptors.
	 * @param file The file to add.
	 */
	void inputToGroup(List<Ext2CreateGroup> *list, Ext2InputFile *file);
    
	/**
	 * Allocate and initialize a superblock.
	 */
	Ext2SuperBlock * initSuperBlock();
	
	/** Program name we are invoked with. */
	char *prog;
	
	/** Path to the output image. */
	char *image;
	
	/** Path to the input directory. */
	char *input;
	
	/** Pointer to the superblock. */
	Ext2SuperBlock *super;
	
	/** Contains all files to be written into the new filesystem. */
	Ext2InputFile *inputRoot;
	
	/** List of file patterns to ignore. */
	List<String> excludes;
	
	/** Size of each block. */
        Size blockSize;
	
	/** Size of a fragment. */
	Size fragmentSize;
};

#endif /* __FILESYSTEM_EXT2CREATE_H */
