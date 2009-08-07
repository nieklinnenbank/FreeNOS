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

#ifndef __FILESYSTEM_EXT2FILESYSTEM_H
#define __FILESYSTEM_EXT2FILESYSTEM_H

#ifndef __HOST__
#include <FileSystem.h>
#include <FileSystemMessage.h>
#include <FileSystemPath.h>
#include <Storage.h>
#include <Types.h>
#include <Error.h>
#include <Array.h>
#include <HashTable.h>
#include <Integer.h>
#include "Ext2SuperBlock.h"
#include "Ext2Inode.h"
#include "Ext2Group.h"
#endif /* __HOST__ */

/**
 * @defgroup ext2 ext2fs (Extended 2 Filesystem)
 * @{
 */

/**
 * @name Special inode numbers
 * @{
 */

/** Bad blocks inode. */
#define	EXT2_BAD_INO		1	

/** Root inode. */
#define EXT2_ROOT_INO		2

/** Boot loader inode. */
#define EXT2_BOOT_LOADER_INO	5	

/** Undelete directory inode. */
#define EXT2_UNDEL_DIR_INO	6

/** First non-reserved inode for old ext2 filesystems */
#define EXT2_GOOD_OLD_FIRST_INO 11

/**
 * @}
 */

/**
 * @name Filesystem limits.
 * @{
 */

/** Maximal count of links to a file. */
#define EXT2_LINK_MAX		32000

/** Minimum blocksize. */
#define EXT2_MIN_BLOCK_SIZE	1024

/** Maximum blocksize. */
#define	EXT2_MAX_BLOCK_SIZE	4096

/** Minimum fragment size. */
#define EXT2_MIN_FRAG_SIZE	1024

/** Maximum fragment size. */
#define	EXT2_MAX_FRAG_SIZE	4096

/** Maximum length of a directory entry name. */
#define EXT2_NAME_LEN 255

/** Size of an Inode in the origional filesystem. */
#define EXT2_GOOD_OLD_INODE_SIZE 128 

/** Maximal mount counts between two filesystem checks. */
#define EXT2_DFL_MAX_MNT_COUNT	20

/** Blocks to reserve for expanding files. */
#define EXT2_DFL_RESERVE_BLOCKS	8

/** Max window size: 1024(direct blocks) + 3([t,d]indirect blocks). */
#define EXT2_MAX_RESERVE_BLOCKS	1027

/** Indicates no blocks are reserved. */
#define EXT2_RESERVE_WINDOW_NOT_ALLOCATED 0

/**
 * @}
 */

/**
 * @name Operating System codes.
 * @{
 */

/** GNU/Linux. */
#define EXT2_OS_LINUX		0

/** GNU/Hurd. */
#define EXT2_OS_HURD		1

/** MASIX Multi-Server Operating System. */
#define EXT2_OS_MASIX		2

/** Free Berkeley Software Distribution. */
#define EXT2_OS_FREEBSD		3

/** Lites Kernel. */
#define EXT2_OS_LITES		4

/** Free Niek's Operating System. */
#define EXT2_OS_FREENOS		5

/**
 * @}
 */

/**
 * @name Revision levels and Versioning
 * @{
 */

/** Date when ext2 was officially created. */
#define EXT2_DATE		"95/08/09"

/** The second extended file system version. */
#define EXT2_VERSION		"0.5b"

/** The good old (original) format. */
#define EXT2_GOOD_OLD_REV	0

/** V2 format with dynamic inode sizes. */
#define EXT2_DYNAMIC_REV	1

/** Current stable revision. */
#define EXT2_CURRENT_REV	EXT2_GOOD_OLD_REV

/** Highest known revision. */
#define EXT2_MAX_SUPP_REV	EXT2_DYNAMIC_REV

/**
 * @}
 */

/**
 * @name Feature definitions.
 * @{
 */

/** Directory preallocation. */
#define EXT2_FEATURE_COMPAT_DIR_PREALLOC	0x0001
#define EXT2_FEATURE_COMPAT_IMAGIC_INODES	0x0002

/** Journalling support. */
#define EXT3_FEATURE_COMPAT_HAS_JOURNAL		0x0004

/** Extended attributes. */
#define EXT2_FEATURE_COMPAT_EXT_ATTR		0x0008

/** Resizable inodes. */
#define EXT2_FEATURE_COMPAT_RESIZE_INO		0x0010

/** Directory indexes. */
#define EXT2_FEATURE_COMPAT_DIR_INDEX		0x0020

/** Wildcard mask. */
#define EXT2_FEATURE_COMPAT_ANY			0xffffffff

#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER	0x0001

/** Large file support. */
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE	0x0002
#define EXT2_FEATURE_RO_COMPAT_BTREE_DIR	0x0004

/** Wildcard mask. */
#define EXT2_FEATURE_RO_COMPAT_ANY		0xffffffff

/** Filesystem compression support. */
#define EXT2_FEATURE_INCOMPAT_COMPRESSION	0x0001
#define EXT2_FEATURE_INCOMPAT_FILETYPE		0x0002
#define EXT3_FEATURE_INCOMPAT_RECOVER		0x0004
#define EXT3_FEATURE_INCOMPAT_JOURNAL_DEV	0x0008
#define EXT2_FEATURE_INCOMPAT_META_BG		0x0010
#define EXT2_FEATURE_INCOMPAT_ANY		0xffffffff

#define EXT2_FEATURE_COMPAT_SUPP		EXT2_FEATURE_COMPAT_EXT_ATTR
#define EXT2_FEATURE_INCOMPAT_SUPP		(EXT2_FEATURE_INCOMPAT_FILETYPE| \
						 EXT2_FEATURE_INCOMPAT_META_BG)
#define EXT2_FEATURE_RO_COMPAT_SUPP		(EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER| \
					         EXT2_FEATURE_RO_COMPAT_LARGE_FILE| \
						 EXT2_FEATURE_RO_COMPAT_BTREE_DIR)
#define EXT2_FEATURE_RO_COMPAT_UNSUPPORTED	~EXT2_FEATURE_RO_COMPAT_SUPP
#define EXT2_FEATURE_INCOMPAT_UNSUPPORTED	~EXT2_FEATURE_INCOMPAT_SUPP

/**
 * @}
 */

/**
 * @name Filesystem states
 * @{
 */

/** Unmounted cleanly. */
#define	EXT2_VALID_FS		0x0001	

/** Errors detected. */
#define	EXT2_ERROR_FS		0x0002

/**
 * @}
 */

/** 
 * @name Mount flags
 * @{
 */

/** Do mount-time checks. */
#define EXT2_MOUNT_CHECK	0x000001

/** Don't use the new Orlov allocator. */
#define EXT2_MOUNT_OLDALLOC	0x000002

/** Create files with directory's group. */
#define EXT2_MOUNT_GRPID	0x000004

/** Some debugging messages. */
#define EXT2_MOUNT_DEBUG	0x000008

/** Continue on errors. */
#define EXT2_MOUNT_ERRORS_CONT	0x000010

/** Remount fs ro on errors. */
#define EXT2_MOUNT_ERRORS_RO	0x000020

/** Panic on errors. */
#define EXT2_MOUNT_ERRORS_PANIC	0x000040

/** Mimics the Minix statfs. */
#define EXT2_MOUNT_MINIX_DF	0x000080

/** No buffer_heads. */
#define EXT2_MOUNT_NOBH		0x000100

/** Disable 32-bit UIDs. */
#define EXT2_MOUNT_NO_UID32	0x000200

/** Extended user attributes. */
#define EXT2_MOUNT_XATTR_USER	0x004000

/** POSIX Access Control Lists. */
#define EXT2_MOUNT_POSIX_ACL	0x008000

/** Execute in place. */
#define EXT2_MOUNT_XIP		0x010000

/** User quota. */
#define EXT2_MOUNT_USRQUOTA	0x020000

/** Group quota. */
#define EXT2_MOUNT_GRPQUOTA	0x040000

/** Preallocation. */
#define EXT2_MOUNT_RESERVATION	0x080000

/**
 * @}
 */

/**
 * @name Behaviour when detecting errors
 * @{
 */

/** Continue execution. */
#define EXT2_ERRORS_CONTINUE	1	

/** Remount fs read-only. */
#define EXT2_ERRORS_RO		2

/** Panic. */
#define EXT2_ERRORS_PANIC	3

/** Default action. */
#define EXT2_ERRORS_DEFAULT	EXT2_ERRORS_CONTINUE

/**
 * @}
 */

#ifndef __HOST__

/**
 * Second Extended FileSystem (ext2).
 * @see http://e2fsprogs.sourceforge.net/ext2.html
 * @see http://ext4.wiki.kernel.org
 */
class Ext2FileSystem : public FileSystem
{
    public:
    
	/**
	 * Class constructor function.
	 * @param path Path to which we are mounted.
	 * @param storage Storage provider.
	 */
	Ext2FileSystem(const char *path, Storage *storage);

	/**
	 * Retrieve the superblock pointer.
	 * @return Pointer to the Ext2SuperBlock for this filesystem.
	 * @see Ext2SuperBlock
	 */
	Ext2SuperBlock * getSuperBlock()
	{
	    return &superBlock;
	}

	/**
	 * Get the underlying Storage object.
	 * @return Storage pointer.
	 * @see Storage
	 */
	Storage * getStorage()
	{
	    return storage;
	}

	/**
	 * Read an ext2 inode from the filesystem.
	 * @param inodeNum Inode number.
	 * @return Pointer to an Ext2Inode on success, ZERO on failure.
	 * @see Ext2Inode
	 */
	Ext2Inode * getInode(u32 inodeNum);

	/**
	 * Read an ext2 group descriptor from the filesystem.
	 * @param groupNum Group descriptor number.
	 * @return Pointer to an Ext2Group on success, ZERO on failure.
	 * @see Ext2Group
	 */
	Ext2Group * getGroup(u32 groupNum);

	/**
	 * Read an ext2 group descriptor from the filesystem.
	 * @param inodeNum Find the corresponding group via this inode number.
	 * @return Pointer to an Ext2Group on success, ZERO on failure.
	 * @see Ext2Group
	 * @see Ext2Inode
	 */
	Ext2Group * getGroupByInode(u32 inodeNum);

	/**
	 * Calculates the offset inside storage for a given block.
	 * @param inode Ext2Inode pointer.
	 * @param blk Calculate the offset for this block.
	 * @return Offset in bytes in storage.
	 * @see Ext2Inode
	 * @see storage
	 */
	u64 getOffset(Ext2Inode *inode, Size blk);

    private:

	/** Provides storage. */
	Storage *storage;
	
	/** Superblock. */
	Ext2SuperBlock superBlock;
	
	/** Group descriptors. */
	Array<Ext2Group> *groups;

	/** Inode cache. */
	HashTable<Integer<u32>,Ext2Inode> inodes;
};

#endif /* __HOST__ */

/**
 * @}
 */

#endif /* __FILESYSTEM_EXT2FILESYSTEM_H */
