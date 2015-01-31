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

#ifndef __FILESYSTEM_EXT2SUPERBLOCK_H
#define __FILESYSTEM_EXT2SUPERBLOCK_H

#include <ByteOrder.h>
#include <Types.h>

/**                                                                                                                                                                                                     
 * @defgroup ext2 Extended 2 Filesystem
 * @{
 */

/** Magic number is used to detect a valid superblock. */
#define EXT2_SUPER_MAGIC 0xEF53

/**
 * @name Superblock macros.
 * @{
 */

/**
 * Retrieve blocksize.
 * @param sb superblock pointer.
 * @return Size of a block.
 * @see Ext2SuperBlock
 */
#define EXT2_BLOCK_SIZE(sb) \
    ((Size)(1024 << (sb)->log2BlockSize))

/**
 * Get the number of addresses per block.
 * @param sb superblock pointer.
 * @return Number of addresses per block.
 * @see Ext2SuperBlock
 */    
#define EXT2_ADDR_PER_BLOCK(sb) \
    (EXT2_BLOCK_SIZE(sb) / sizeof (u32))

/**
 * Block size in bits?
 * @param sb superblock pointer.
 * @return Block size in bits?
 * @see Ext2SuperBlock
 */    
#define EXT2_BLOCK_SIZE_BITS(sb) \
    ((sb)->log2BlockSize + 10)

/**
 * Retrieve size of an inode.
 * @param sb superblock pointer.
 * @return Size of a inode.
 * @see Ext2SuperBlock
 * @see Ext2Inode
 */
#define EXT2_INODE_SIZE(sb) \
    (((sb)->majorRevision == EXT2_GOOD_OLD_REV) ? \
	EXT2_GOOD_OLD_INODE_SIZE : (sb)->dynamicInodeSize)

/**
 * Get the first inode.
 * @param sb superblock pointer.
 * @return First inode.
 * @see Ext2SuperBlock
 * @see Ext2Inode
 */
#define EXT2_FIRST_INO(sb) \
    (((sb)->majorRevision == EXT2_GOOD_OLD_REV) ? \
	EXT2_GOOD_OLD_FIRST_INO : (sb)->dynamicFirstInode)

/**
 * Get the size of fragments.
 * @param sb superblock pointer.
 * @return Fragments size.
 * @see Ext2SuperBlock
 */
#define EXT2_FRAG_SIZE(sb) \
    (EXT2_MIN_FRAG_SIZE << (sb)->log2FragmentSize)

/**
 * Get the number of fragments per block.
 * @param sb superblock pointer.
 * @return Fragments per block.
 * @see Ext2SuperBlock
 */
#define EXT2_FRAGS_PER_BLOCK(sb) \
    (EXT2_BLOCK_SIZE(s) / EXT2_FRAG_SIZE(s))

/**
 * Calculate the number of group descriptors.
 * @param sb superblock pointer.
 * @return Number of groups.
 * @see Ext2SuperBlock
 * @see Ext2Group
 */
#define EXT2_GROUPS_COUNT(sb) \
    (((le32_to_cpu((sb)->blocksCount) - \
       le32_to_cpu((sb)->firstDataBlock) - 1) \
       / EXT2_BLOCKS_PER_GROUP(sb)) + 1)


/**
 * Retrieve the offset of the superblock on storage.
 * @return Offset of the Ext2SuperBlock on storage.
 * @see Ext2SuperBlock
 * @see Storage
 */
#define EXT2_SUPER_OFFSET (0x400)

/**
 * @}
 */

/**
 * Second Extended FileSystem (ext2) super block.
 */
typedef struct Ext2SuperBlock
{
    le32 inodesCount;		/**< Total number of inodes. */
    le32 blocksCount;		/**< Total number of data blocks. */
    le32 reservedBlocksCount;	/**< Blocks reserved for the superuser. */
    le32 freeBlocksCount;	/**< Number of free data blocks. */
    le32 freeInodesCount;	/**< Free inodes remaining. */
    le32 firstDataBlock;	/**< First data block. */
    le32 log2BlockSize;		/**< Size of each data block, log base 2. */
    le32 log2FragmentSize;	/**< Size of a fragment, log base 2. */
    le32 blocksPerGroup;	/**< Number of blocks per group. */
    le32 fragmentsPerGroup;	/**< Number of fragments per group. */
    le32 inodesPerGroup;	/**< Number of inodes per group. */
    le32 mountTime;		/**< Last time we where mounted. */
    le32 writeTime;		/**< Timestamp of the last write. */
    le16 mountCount;		/**< Number of times we where mounted. */
    le16 maximumMountCount;	/**< Maximum mounts before fsck is enforced. */
    le16 magic;			/**< Allows detection of valid superblocks. */
    le16 state;			/**< Describes the current status. */
    le16 errors;		/**< Behaviour when detecting errors. */
    le16 minorRevision;		/**< Filesystem minor revision level. */
    le32 lastCheck;		/**< Timestamp of the last check. */
    le32 checkInterval;		/**< Maximum time between checks. */
    le32 creatorOS;		/**< Identify of the creator OS. */
    le32 majorRevision;		/**< Filesystem major revision level. */
    le16 defaultReservedUid;	/**< Default uid for reserved blocks. */
    le16 defaultReservedGid;	/**< Default gid for reserved blocks. */

    /**
     * @name EXT2_DYNAMIC_REV superblocks only.
     *
     * The difference between the compatible feature set and
     * the incompatible feature set is that if there is a bit set
     * in the incompatible feature set that the kernel doesn't
     * know about, it should refuse to mount the filesystem.
     *
     * e2fsck's requirements are more strict; if it doesn't know
     * about a feature in either the compatible or incompatible
     * feature set, it must abort and not try to meddle with
     * things it doesn't understand...
     *
     * @{
     */

    le32 dynamicFirstInode;	/**< First non-reserved inode. */
    le16 dynamicInodeSize;	/**< Size of Inode structure. */
    le16 blockGroupNumber;	/**< Block group numberof this superblock. */
    le32 compatibleFeatures;	/**< Compatible feature set. */
    le32 incompatibleFeatures;	/**< Incompatible feature set. */
    le32 roCompatibleFeatures;	/**< Readonly-compatible feature set. */
      u8 uuid[16];		/**< 128-bit uuid for volume. */
      s8 volumeName[16];	/**< Volume name. */
      s8 lastMounted[64];	/**< Directory where last mounted. */
    le32 algorithmUsageBitmap;	/**< For compression (unused in linux?). */

    /**
     * @}
     */

    /**
     * @name Performance hints.
     * 
     * Directory preallocation should only happen if
     * the EXT2_COMPAT_PREALLOC flag is on.
     *
     * @{
     */

    u8   preallocBlocks;	/**< Number of blocks to try to preallocate. */
    u8   preallocDirBlocks;	/**< Blocks to preallocate for dirs. */
    u16  padding1;
    
    /**
     * @}
     */
    
   /**
    * @name Journaling support.
    *
    * Valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
    * 
    * @{
    */
    
    u8   journalUuid[16];	/**< Uuid of journal superblock. */
    u32  journalInode;		/**< Inode number of journal file. */
    u32  journalDevice;		/**< Device number of journal file. */
    u32  lastOrphan;		/**< Start of list of inodes to delete. */
    u32  hashSeed[4];		/**< HTREE hash seed. */
    u8   defaultHashVersion;	/**< Default hash version to use. */
    u8   reservedCharPad;
    u16  reservedWordPad;
    le32 default_mount_opts;
    le32 first_meta_bg;		/**< First metablock block group. */
    u32  reserved[190];		/**< Padding to the end of the block. */

    /**
     * @}
     */
}
Ext2SuperBlock;

/**
 * @}
 */

#endif /* __FILESYSTEM_EXT2SUPERBLOCK_H */
