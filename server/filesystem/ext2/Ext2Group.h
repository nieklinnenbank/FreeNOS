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

#ifndef __FILESYSTEM_EXT2GROUP_H
#define __FILESYSTEM_EXT2GROUP_H

#include <Types.h>

/**
 * @defgroup ext2 ext2fs (Extended 2 Filesystem)
 * @{
 */

/**
 * @name Group macros
 * 
 * Macro-instructions used to manage group descriptors
 *
 * @{
 */

/**
 * Get the number of blocks per Ext2Group.
 * @param s superblock pointer.
 * @return Number of blocks per group.
 * @see Ext2Group
 * @see Ext2SuperBlock
 */
#define EXT2_BLOCKS_PER_GROUP(sb) \
    ((sb)->blocksPerGroup)

/**
 * Retrieve the maximum number of group descriptors per block.
 * @param s superblock pointer.
 * @return Maximum number of Ext2Group descriptors per block.
 * @see Ext2Group
 * @see Ext2SuperBlock
 */
#define EXT2_GROUPS_PER_BLOCK(sb) \
    (EXT2_BLOCK_SIZE(sb) / sizeof(Ext2Group))

/**
 * Get the number of inodes per Ext2Group.
 * @param s superblock pointer.
 * @return Number of inodes per group.
 * @see Ext2Inode
 * @see Ext2Group
 * @see Ext2SuperBlock
 */
#define EXT2_INODES_PER_GROUP(sb) \
    ((sb)->inodesPerGroup)

/**
 * @}
 */

/**
 * Structure of a blocks group descriptor.
 */
typedef struct Ext2Group
{
    le32 blockBitmap;		/**< Blocks bitmap block. */
    le32 inodeBitmap;		/**< Inodes bitmap block. */
    le32 inodeTable;		/**< Inodes table block. */
    le16 freeBlocksCount;	/**< Free blocks count. */
    le16 freeInodesCount;	/**< Free inodes count. */
    le16 usedDirsCount;		/**< Directories count. */
    le16 pad;			/**< Padding bits. */
    le32 reserved[3];		/**< Reserved bits. */
}
Ext2Group;

/**
 * @}
 */

#endif /* __FILESYSTEM_EXT2GROUP_H */
