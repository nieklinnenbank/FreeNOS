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

#ifndef __FILESYSTEM_EXT2INODE_H
#define __FILESYSTEM_EXT2INODE_H

#ifndef __HOST__
#include <FileSystemMessage.h>
#endif /* __HOST__ */

/**                                                                                                                                                                                                     
 * @defgroup ext2 ext2fs (Extended 2 Filesystem)
 * @{
 */

/**
 * @name Inode (in)direct block counts.
 * @{
 */

/** Number of direct blocks. */
#define	EXT2_NDIR_BLOCKS	12

/** Indirect blocks. */
#define	EXT2_IND_BLOCK		(EXT2_NDIR_BLOCKS)

/** Double indirect blocks. */
#define	EXT2_DIND_BLOCK		(EXT2_IND_BLOCK  + 1)

/** Triple indirect blocks. */
#define	EXT2_TIND_BLOCK		(EXT2_DIND_BLOCK + 1)

/** Total number of blocks in an Ext2Inode. */
#define	EXT2_N_BLOCKS		(EXT2_TIND_BLOCK + 1)

/**
 * @}
 */

/**
 * @name Inode flags
 * @{
 */

/** Secure deletion. */
#define	EXT2_SECRM_FL		0x00000001

/** Undelete. */
#define	EXT2_UNRM_FL		0x00000002

/** Compress file. */
#define	EXT2_COMPR_FL		0x00000004

/** Synchronous updates. */
#define EXT2_SYNC_FL		0x00000008

/** Immutable file. */
#define EXT2_IMMUTABLE_FL	0x00000010

/** Writes to file may only append. */
#define EXT2_APPEND_FL		0x00000020

/** Do not dump file. */
#define EXT2_NODUMP_FL		0x00000040

/** Do not update atime. */
#define EXT2_NOATIME_FL		0x00000080

/** Btree format dir. */
#define EXT2_BTREE_FL		0x00001000

/** Hash-indexed directory. */
#define EXT2_INDEX_FL		0x00001000

/** AFS directory. */
#define EXT2_IMAGIC_FL		0x00002000

/** File tail should not be merged. */
#define EXT2_NOTAIL_FL		0x00008000

/** Dirsync behaviour (directories only). */
#define EXT2_DIRSYNC_FL		0x00010000

/** Top of directory hierarchies. */
#define EXT2_TOPDIR_FL		0x00020000

/** Reserved for ext2 lib. */
#define EXT2_RESERVED_FL	0x80000000

/** User visible flags. */
#define EXT2_FL_USER_VISIBLE	0x0003DFFF

/** User modifiable flags. */
#define EXT2_FL_USER_MODIFIABLE	0x000380FF

/**
 * @}
 */

/**
 * @name Inode macros
 * @{
 */

/**
 * Retrieve the FileType of an Ext2Inode.
 * @param i Ext2Inode pointer.
 * @return FileType value.
 * @see dirent.h
 * @see stat.h
 */
#define EXT2_FILETYPE(i) \
    ({ \
	FileType types[] = \
	{ \
	    UnknownFile, \
	    FIFOFile, \
	    CharacterDeviceFile, \
	    UnknownFile, \
	    DirectoryFile, \
	    UnknownFile, \
	    BlockDeviceFile, \
	    UnknownFile, \
	    RegularFile, \
	    UnknownFile, \
	    SymlinkFile, \
	    UnknownFile, \
	    SocketFile, \
	}; \
	types[((i)->mode & 0170000) >> 12]; \
    })

/**
 * @}
 */

/**
 * Structure of an ext2 inode on the disk
 */
typedef struct Ext2Inode
{
    le16 mode;		/**< File mode. */	
    le16 uid;		/**< Low 16 bits of Owner Uid. */
    le32 size;		/**< Size in bytes. */
    le32 atime;		/**< Access time. */
    le32 ctime;		/**< Creation time. */
    le32 mtime;		/**< Modification time. */
    le32 dtime;		/**< Deletion Time. */
    le16 gid;		/**< Low 16 bits of Group Id. */
    le16 linksCount;	/**< Links count. */
    le32 blocks;	/**< Number of 512-byte blocks. */
    le32 flags;		/**< File flags. */
    le32 reserved;	/**< Reserved bits. */
    le32 block[EXT2_N_BLOCKS]; /**< Pointers to blocks. */
    le32 generation;	/**< File version (for NFS). */
    le32 fileACL;	/**< File ACL. */
    le32 directoryACL;	/**< Directory ACL. */
    le32 fragmentAddr;	/**< Fragment address. */
    u8	fragmentNumber;	/**< Fragment number. */
    u8	fragmentSize;	/**< Fragment size. */
    u16	padding;	/**< Padding bytes. */
    le16 uidHigh;	/**< High 16 bits of Owner Uid. */
    le16 gidHigh;	/**< High 16 bits of Group Id. */
    u32	reserved2;	/**< Reserved. */
}
Ext2Inode;

/**
 * @}
 */

#endif /* __FILESYSTEM_EXT2INODE_H */
