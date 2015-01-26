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

#include <Types.h>
#include <FileStorage.h>
#include <BootModule.h>
#include "LinnFileSystem.h"
#include "LinnInode.h"
#include "LinnFile.h"
#include "LinnDirectory.h"
#include <stdlib.h>
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
	BootModule *bm = new BootModule("/boot.linn.gz");
	
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
	LinnFileSystem server(path, storage);
	
	if (server.mount(background))
	{
    	    return server.run();
	}
    }
    return EXIT_FAILURE;
}

LinnFileSystem::LinnFileSystem(const char *p, Storage *s)
    : FileSystem(p), storage(s), groups(ZERO)
{
    LinnInode *rootInode;
    LinnGroup *group;
    Size offset;
    Error e;

    /* Open the system log. */
    openlog("LinnFS", LOG_PID, LOG_USER);

    /* Read out the superblock. */
    if ((e = s->read(LINN_SUPER_OFFSET, &super,
		     sizeof(super))) <= 0)
    {
	syslog(LOG_ERR, "reading superblock failed: %s",
	       strerror(e));
	exit(EXIT_FAILURE);
    }
    /* Verify magic. */
    if (super.magic0 != LINN_SUPER_MAGIC0 ||
        super.magic1 != LINN_SUPER_MAGIC1)
    {
	syslog(LOG_ERR, "magic mismatch");
	exit(EXIT_FAILURE);
    }
    /* Create groups vector. */
    groups = new Array<LinnGroup>(LINN_GROUP_COUNT(&super));

    /* Read out group descriptors. */
    for (Size i = 0; i < LINN_GROUP_COUNT(&super); i++)
    {
	/* Allocate buffer. */
	group  = new LinnGroup;
	offset = (super.groupsTable * super.blockSize) +
		 (sizeof(LinnGroup)  * i);

	/* Read from storage. */
	if ((e = s->read(offset, group, sizeof(LinnGroup))) <= 0)
	{
	    syslog(LOG_ERR, "reading group descriptor failed: %s",
		   strerror(e));
	    exit(EXIT_FAILURE);
	}
	/* Insert in the groups vector. */
	groups->insert(i, group);
    }
    syslog(LOG_INFO, "%d group descriptors",
	   LINN_GROUP_COUNT(&super));
    
    /* Print out superblock information. */
    syslog(LOG_INFO, "%d inodes, %d blocks",
	   super.inodesCount - super.freeInodesCount,
	   super.blocksCount - super.freeBlocksCount);

    /* Read out the root directory. */
    rootInode = getInode(LINN_INODE_ROOT);
    setRoot(new LinnDirectory(this, rootInode));
    
    /* Done. */
    syslog(LOG_INFO, "mounted as '%s'", p);
}

LinnInode * LinnFileSystem::getInode(u32 inodeNum)
{
    LinnGroup *group;
    LinnInode *inode;
    Size offset;
    Error e;
    Integer<u32> inodeInt = inodeNum;
    
    /* Validate the inode number. */
    if (inodeNum >= super.inodesCount)
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
    inode  = new LinnInode;
    offset = (group->inodeTable * super.blockSize) +
    	    ((inodeNum % super.inodesPerGroup) * sizeof(LinnInode));
	     
    /* Read inode from storage. */
    if ((e = storage->read(offset, inode, sizeof(LinnInode))) <= 0)
    {
        syslog(LOG_ERR, "reading inode failed: %s",
	       strerror(e));
	return ZERO;
    }
    /* Insert into the cache. */
    inodes.insert(new Integer<u32>(inodeNum), inode);
    return inode;
}

LinnGroup * LinnFileSystem::getGroup(u32 groupNum)
{
    return (*groups)[groupNum];
}

LinnGroup * LinnFileSystem::getGroupByInode(u32 inodeNum)
{
    return getGroup(inodeNum ? inodeNum / super.inodesPerGroup : 0);
}

u64 LinnFileSystem::getOffset(LinnInode *inode, u32 blk)
{
    u64 numPerBlock = LINN_SUPER_NUM_PTRS(&super), offset;
    u32 *block = ZERO;
    Size depth = ZERO, remain = 1;

    /* Direct blocks. */
    if (blk < LINN_INODE_DIR_BLOCKS)
    {
	return inode->block[blk] * super.blockSize;
    }
    /* Indirect blocks. */
    if (blk - LINN_INODE_DIR_BLOCKS < numPerBlock)
    {
	depth = 1;
    }
    /* Double indirect blocks. */
    else if (blk - LINN_INODE_DIR_BLOCKS < numPerBlock * numPerBlock)
    {
	depth = 2;
    }
    /* Triple indirect blocks. */
    else
	depth = 3;
    
    /* Allocate temporary block. */
    block   = new u32[LINN_SUPER_NUM_PTRS(&super)];
    offset  = inode->block[(LINN_INODE_DIR_BLOCKS + depth - 1)];
    offset *= super.blockSize;
    
    /* Lookup the block number. */
    while (true)
    {
	/* Fetch block. */
	if (storage->read(offset, block, super.blockSize) < 0)
	{
	    delete block;
	    return 0;
	}
	/* Calculate the number of blocks remaining per entry. */
	for (Size i = 0; i < depth - 1; i++)
	{
	    remain *= LINN_SUPER_NUM_PTRS(&super);
	}
	/* More indirection? */
	if (remain == 1)
	{
	    break;
	}
	/* Calculate the next offset. */
	offset  = block[ (blk - LINN_INODE_DIR_BLOCKS) / remain ];
	offset *= super.blockSize;
	remain  = 1;
	depth--;
    }
    /* Calculate the final offset. */
    offset  = block[ (blk - LINN_INODE_DIR_BLOCKS) %
		      LINN_SUPER_NUM_PTRS(&super) ];
    offset *= super.blockSize;
    
    /* All done. */
    delete block;
    return offset;	
}
