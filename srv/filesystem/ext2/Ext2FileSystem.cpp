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

#include <File.h>
#include <BootModule.h> 
#include <FileStorage.h>
#include <Directory.h>
#include <Device.h>
#include "Ext2FileSystem.h"
#include "Ext2SuperBlock.h"
#include "Ext2File.h"
#include "Ext2Directory.h"
#include "Ext2Inode.h"
#include "Ext2Group.h"
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

int main(int argc, char **argv)
{
    Storage *storage = ZERO;
    bool background  = false;
    const char *path = "/";
    
    /* 
     * Mount the given file, or use the default GRUB boot module. 
     */
    if (argc > 3)
    {
        storage    = new FileStorage(argv[1], atoi(argv[2]));
        background = true;
        path       = argv[3];
    }
    else
    {
        BootModule *bm = new BootModule("/boot/boot.ext2.gz");
    
        if (bm->load())
        {
            storage = bm;
        }
    }
    
    /* 
     * Mount, then start serving requests. 
     */
    if (storage)
    {
        Ext2FileSystem server(path, storage);

        if (server.mount(background))
        {
            return server.run();
        }
    }
    return EXIT_FAILURE;
}

Ext2FileSystem::Ext2FileSystem(const char *p, Storage *s)
    : FileSystem(p), storage(s), groups(ZERO)
{
    Ext2Inode *rootInode;
    Ext2Group *group;
    Size offset;
    Error e;

    /* Open the system logs. */
    openlog("Ext2FS", LOG_PID, LOG_USER);

    /* Read out the superblock. */
    if ((e = s->read(EXT2_SUPER_OFFSET, &superBlock,
		     sizeof(superBlock))) <= 0)
    {
	syslog(LOG_ERR, "reading superblock failed: %s",
	       strerror(e));
	exit(EXIT_FAILURE);
    }
    /* Verify magic. */
    if (superBlock.magic != EXT2_SUPER_MAGIC)
    {
	syslog(LOG_ERR, "%x != EXT2_SUPER_MAGIC",
	     superBlock.magic);
	exit(EXIT_FAILURE);
    }
    /* Create groups vector. */
    groups  = new Array<Ext2Group>(EXT2_GROUPS_COUNT(&superBlock));

    /* Read out group descriptors. */
    for (Size i = 0; i < EXT2_GROUPS_COUNT(&superBlock); i++)
    {
	/* Allocate buffer. */
	group   = new Ext2Group;
	offset  = le32_to_cpu(superBlock.firstDataBlock ?
			      superBlock.firstDataBlock + 1 : 1) *
	          EXT2_BLOCK_SIZE(&superBlock);
	offset += sizeof(Ext2Group) * i;

	/* Read from storage. */
	if ((e = s->read(offset, group, sizeof(Ext2Group))) <= 0)
	{
	    syslog(LOG_ERR, "reading group descriptor failed: %s",
		   strerror(e));
	    exit(EXIT_FAILURE);
	}
	/* Insert in the groups vector. */
	groups->insert(i, group);
    }
    syslog(LOG_INFO, "%d group descriptors",
	 EXT2_GROUPS_COUNT(&superBlock));
    
    /* Print out superblock information. */
    syslog(LOG_INFO, "%d inodes, %d blocks",
	   superBlock.inodesCount, superBlock.blocksCount);

    /* Read out the root directory. */
    rootInode = getInode(EXT2_ROOT_INO);
    setRoot(new Ext2Directory(this, rootInode));
    syslog(LOG_INFO, "mounted as '%s'", p);
}

Ext2Inode * Ext2FileSystem::getInode(u32 inodeNum)
{
    Ext2Group *group;
    Ext2Inode *inode;
    Size offset;
    Error e;
    Integer<u32> inodeInt = inodeNum;
    
    /* Validate the inode number. */
    if ((inodeNum != EXT2_ROOT_INO && inodeNum < EXT2_FIRST_INO(&superBlock)) ||
        (inodeNum > superBlock.inodesCount))
    {
	return ZERO;
    }
    /* Do we have this Inode cached already? */
    if ((inode = inodes[&inodeInt]))
    {
	return inode;
    }
    /* Get the group descriptor. */
    if (!(group = getGroupByInode(inodeNum)))
    {
	return ZERO;
    }
    /* Allocate inode buffer. */
    inode   = new Ext2Inode;
    offset  = ((inodeNum - 1) % EXT2_INODES_PER_GROUP(&superBlock))
	      * EXT2_INODE_SIZE(&superBlock);
    offset += le32_to_cpu(group->inodeTable) * EXT2_BLOCK_SIZE(&superBlock);
	     
    /* Read inode from storage. */
    if ((e = storage->read(offset, inode, sizeof(Ext2Inode))) <= 0)
    {
        syslog(LOG_ERR, "reading inode failed: %s",
	       strerror(e));
	return ZERO;
    }
    /* Insert into the cache. */
    inodes.insert(new Integer<u32>(inodeNum), inode);
    return inode;
}

Ext2Group * Ext2FileSystem::getGroup(u32 groupNum)
{
    return (*groups)[groupNum];
}

Ext2Group * Ext2FileSystem::getGroupByInode(u32 inodeNum)
{
    return getGroup((inodeNum - 1) / EXT2_INODES_PER_GROUP(&superBlock));
}

u64 Ext2FileSystem::getOffset(Ext2Inode *inode, Size blk)
{
    Size numPerBlock = EXT2_ADDR_PER_BLOCK(&superBlock);
    Size depth = 0, remain = 1;
    u32 *block;
    u64 offset;

    /* Direct blocks. */
    if (blk < EXT2_NDIR_BLOCKS)
    {
	return inode->block[blk] * EXT2_BLOCK_SIZE(&superBlock);
    }
    /* Indirect blocks. */
    if (blk - EXT2_NDIR_BLOCKS < numPerBlock)
    {
	depth = 1;
    }
    /* Double indirect blocks. */
    else if (blk - EXT2_NDIR_BLOCKS < numPerBlock * numPerBlock)
    {
	depth = 2;
    }
    /* Tripple indirect blocks. */
    else
	depth = 3;
    
    /* Allocate temporary block. */
    block   = new u32[EXT2_ADDR_PER_BLOCK(&superBlock)];
    offset  = inode->block[(EXT2_NDIR_BLOCKS + depth - 1)];
    offset *= EXT2_BLOCK_SIZE(&superBlock);
    
    /* Lookup the block number. */
    while (depth > 0)
    {
	/* Fetch block. */
	if (storage->read(offset, block,
			  EXT2_BLOCK_SIZE(&superBlock)) < 0)
	{
	    delete block;
	    return 0;
	}
	/* Calculate the number of blocks remaining per entry. */
        for (Size i = 0; i < depth - 1; i++)
        {
            remain *= EXT2_ADDR_PER_BLOCK(&superBlock);
        }
        /* More indirection? */
        if (remain == 1)
        {
            break;
        }
	/* Calculate the next offset. */
	offset  = block[ (blk - EXT2_NDIR_BLOCKS) / remain ];
        offset *= EXT2_BLOCK_SIZE(&superBlock);
	remain  = 1;
        depth--;
    }
    /* Calculate the final offset. */
    offset  = block[ (blk - EXT2_NDIR_BLOCKS) %
		      EXT2_ADDR_PER_BLOCK(&superBlock) ];
    offset *= EXT2_BLOCK_SIZE(&superBlock);
    
    /* All done. */
    delete block;
    return offset;	
}
