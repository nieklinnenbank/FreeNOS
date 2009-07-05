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

#ifndef __FILESYSTEM_LINNINODE_H
#define __FILESYSTEM_LINNINODE_H

/**
 * @defgroup linn Linnenbank Filesystem (LinnFS)
 * @{
 */

/**
 * @name Inode (in)direct block counts.
 * @{
 */

/** Number of direct blocks. */
#define	LINN_NDIR_BLOCKS	12

/** Indirect blocks. */
#define	LINN_IND_BLOCK		(LINN_NDIR_BLOCKS)

/** Double indirect blocks. */
#define	LINN_DIND_BLOCK		(LINN_IND_BLOCK  + 1)

/** Triple indirect blocks. */
#define	LINN_TIND_BLOCK		(LINN_DIND_BLOCK + 1)

/** Total number of blocks in an Ext2Inode. */
#define	LINN_INODE_BLOCKS	(LINN_TIND_BLOCK + 1)

/**
 * @}
 */

/**
 * Structure of an inode on the disk in the LinnFS filesystem.
 */
typedef struct LinnInode
{
    le16 mode;	  /**< POSIX file mode. */
    le16 uid;	  /**< User Identity. */
    le16 gid;	  /**< Group Identity. */
    le64 size;	  /**< Size in bytes. */
    le64 atime;	  /**< Access time. */
    le64 ctime;	  /**< Creation time. */
    le64 mtime;	  /**< Modification time. */
    le16 links;	  /**< Links count. */
    le64 block[LINN_INODE_BLOCKS]; /**< Pointers to blocks. */
    le16 padding; /**< Padding for 64 bytes. */
}
LinnInode;

/**
 * @}
 */

#endif /* __FILESYSTEM_LINNINODE_H */
