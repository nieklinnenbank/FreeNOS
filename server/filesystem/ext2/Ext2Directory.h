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

/*
 *  linux/include/linux/ext2_fs.h
 *
 * Copyright (C) 1992, 1993, 1994, 1995
 * Remy Card (card@masi.ibp.fr)
 * Laboratoire MASI - Institut Blaise Pascal
 * Universite Pierre et Marie Curie (Paris VI)
 *
 *  from
 *
 *  linux/include/linux/minix_fs.h
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#ifndef __FILESYSTEM_EXT2DIRECTORY_H
#define __FILESYSTEM_EXT2DIRECTORY_H

#ifndef __HOST__
#include <FileSystemMessage.h>
#include <Directory.h>
#include <Types.h>
#include "Ext2FileSystem.h"
#include "Ext2Inode.h"
#include "IOBuffer.h"
#endif /* __HOST__ */

/**                                                                                                                                                                                                     
 * @defgroup ext2 ext2fs (Extended 2 Filesystem)
 * @{
 */

/**
 * @name Directory entry macros
 * @{
 */

/**
 * @brief Defines the directory entries boundaries.
 * @note It must be a multiple of 4.
 */
#define EXT2_DIR_PAD		 	4
#define EXT2_DIR_ROUND 			(EXT2_DIR_PAD - 1)
#define EXT2_DIR_REC_LEN(name_len)	(((name_len) + 8 + EXT2_DIR_ROUND) & \
					 ~EXT2_DIR_ROUND)
#define EXT2_MAX_REC_LEN		((1<<16)-1)

/**
 * @}
 */

/**
 * @brief Structure of an ext2 directory entry.
 */
typedef struct Ext2DirectoryEntry
{
    /** Inode number. */
    le32 inode;

    /** Directory entry length. Must be 4-byte aligned? */
    le16 recordLength;
    
    /** Name length. */
    le16 nameLength;

    /** File name. */
    char name[EXT2_NAME_LEN];	
}
Ext2DirectoryEntry;

/**
 * @brief Structure of an ext2 directory entry.
 *
 * The new version of the directory entry.  Since EXT2 structures are
 * stored in intel byte order, and the name_len field could never be
 * bigger than 255 chars, it's safe to reclaim the extra byte for the
 * file_type field.
 *
 * @see Ext2DirectoryEntry
 */
typedef struct Ext2DirectoryEntry2
{
    /** Inode number. */
    le32 inode;

    /** Directory entry length. */
    le16 recordLength;

    /** Name length. */
    u8 nameLength;

    /** Type of file. */
    u8 fileType;

    /** File name. */
    char name[EXT2_NAME_LEN];
}
Ext2DirectoryEntry2;

/**
 * @name File types
 * @{
 */

/**
 * @brief Ext2 directory file types.
 * Only the low 3 bits are used. The
 * other bits are reserved for now.
 */
enum
{
    EXT2_FT_UNKNOWN   = 0,
    EXT2_FT_REG_FILE  = 1,
    EXT2_FT_DIR       = 2,
    EXT2_FT_CHRDEV    = 3,
    EXT2_FT_BLKDEV    = 4,
    EXT2_FT_FIFO      = 5,
    EXT2_FT_SOCK      = 6,
    EXT2_FT_SYMLINK   = 7,
    EXT2_FT_MAX       = 8
};

/**
 * @}
 */

#ifndef __HOST__

/**
 * Extended 2 Filesystem directory.
 * @see Directory
 * @see Ext2DirectoryEntry
 * @see Ext2FileSystem
 */
class Ext2Directory : public Directory
{
    public:
    
	/**
	 * Constructor function.
	 * @param fs Filesystem pointer.
	 * @param inode Inode pointer.
	 * @see Ext2FileSystem
	 * @see Ext2Inode
	 */
	Ext2Directory(Ext2FileSystem *fs, Ext2Inode *inode);

        /** 
         * @brief Read directory entries. 
         * @param buffer Input/Output buffer to output bytes to. 
         * @param size Number of bytes to read, at maximum. 
         * @param offset Offset inside the file to start reading. 
         * @return Number of bytes read on success, Error on failure. 
         */
	Error read(IOBuffer *buffer, Size size, Size offset);

        /** 
         * @brief Retrieves a File pointer for the given entry name. 
         * 
         * This function reads a file from disk corresponding 
         * to the Ext2Inode of the given entry name. It returns 
         * an File object associated with that LinnInode. 
         * 
         * @param name Name of the entry to lookup. 
         * @return Pointer to a File if found, or ZERO otherwise. 
         * 
         * @see File 
         * @see Ext2Inode 
         */
        File * lookup(const char *name);

    private:

	/**
	 * Retrieve a directory entry.
	 * @param dent Ext2DirectoryEntry buffer pointer.
	 * @param name Unique name of the entry.
	 * @return True if found, false otherwise.
	 */
	bool getExt2DirectoryEntry(Ext2DirectoryEntry *dent,
				   const char *name);

	/** Filesystem pointer. */
	Ext2FileSystem *ext2;

	/** Inode which describes the directory. */
	Ext2Inode *inode;
};

#endif /* __HOST__ */

/**
 * @}
 */

#endif /* __FILESYSTEM_EXT2DIRECTORY_H */
