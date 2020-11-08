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
#include <Assert.h>
#include "LinnFileSystem.h"
#include "LinnInode.h"
#include "LinnFile.h"
#include "LinnDirectory.h"

LinnFileSystem::LinnFileSystem(const char *p, Storage *s)
    : FileSystemServer(ZERO, p), storage(s), groups(ZERO)
{
    LinnInode *rootInode;
    LinnGroup *group;
    Size offset;
    FileSystem::Result e;

    // Read out the superblock.
    if ((e = s->read(LINN_SUPER_OFFSET, &super,
                     sizeof(super))) != FileSystem::Success)
    {
        FATAL("reading superblock failed: result = " << (int) e);
    }
    // Verify magic.
    if (super.magic0 != LINN_SUPER_MAGIC0 ||
        super.magic1 != LINN_SUPER_MAGIC1)
    {
        FATAL("magic mismatch");
    }
    // Create groups vector.
    groups = new Vector<LinnGroup *>(LINN_GROUP_COUNT(&super));
    assert(groups != NULL);
    groups->fill(ZERO);

    // Read out group descriptors.
    for (Size i = 0; i < LINN_GROUP_COUNT(&super); i++)
    {
        // Allocate buffer.
        group  = new LinnGroup;
        assert(group != NULL);
        offset = (super.groupsTable * super.blockSize) +
                 (sizeof(LinnGroup)  * i);

        // Read from storage.
        if ((e = s->read(offset, group, sizeof(LinnGroup))) != FileSystem::Success)
        {
            FATAL("reading group descriptor failed: result = " << (int) e);
        }
        // Insert in the groups vector.
        groups->insert(i, group);
    }
    // Print out superblock information.

    INFO(LINN_GROUP_COUNT(&super) << " group descriptors");
    INFO(super.inodesCount - super.freeInodesCount << " inodes, " <<
         super.blocksCount - super.freeBlocksCount << " blocks");

    // Read out the root directory.
    rootInode = getInode(LINN_INODE_ROOT);
    LinnDirectory *dir = new LinnDirectory(this, LINN_INODE_ROOT, rootInode);
    assert(dir != NULL);
    setRoot(dir);

    // Done.
    NOTICE("mounted at " << p);
}

LinnInode * LinnFileSystem::getInode(u32 inodeNum)
{
    LinnGroup *group;
    LinnInode *inode;
    Size offset;
    FileSystem::Result e;

    // Validate the inode number.
    if (inodeNum >= super.inodesCount)
    {
        return ZERO;
    }
    // Do we have this Inode cached already?
    if (inodes.contains(inodeNum))
    {
        return inodes.value(inodeNum);
    }
    // Get the group descriptor.
    if (!(group = getGroupByInode(inodeNum)))
    {
        return ZERO;
    }
    // Allocate inode buffer.
    inode  = new LinnInode;
    assert(inode != NULL);
    offset = (group->inodeTable * super.blockSize) +
                ((inodeNum % super.inodesPerGroup) * sizeof(LinnInode));

    // Read inode from storage.
    if ((e = storage->read(offset, inode, sizeof(LinnInode))) != FileSystem::Success)
    {
        ERROR("reading inode failed: result = " << (int) e);
        return ZERO;
    }
    // Insert into the cache.
    inodes.insert(inodeNum, inode);
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

u64 LinnFileSystem::getOffsetRange(const LinnInode *inode,
                                   const u32 blk,
                                   Size & numContiguous)
{
    static u32 block[LINN_MAX_BLOCK_SIZE / sizeof(u32)];
    const u64 numPerBlock = LINN_SUPER_NUM_PTRS(&super);
    const Size numBlocks = LINN_INODE_NUM_BLOCKS(&super, inode);
    Size depth = ZERO, remain = 1;
    u64 offset;

    assert(LINN_SUPER_NUM_PTRS(&super) <= sizeof(block) / sizeof(u32));

    // Direct blocks.
    if (blk < LINN_INODE_DIR_BLOCKS)
    {
        const u32 offsetBlock = inode->block[blk];
        numContiguous = 1;

        for (Size i = blk + 1; i < numBlocks && i < LINN_INODE_DIR_BLOCKS; i++)
        {
            if (inode->block[i] == offsetBlock + numContiguous)
                numContiguous++;
            else
                break;
        }

        return offsetBlock * super.blockSize;
    }
    // Indirect blocks.
    if (blk - LINN_INODE_DIR_BLOCKS < numPerBlock)
    {
        depth = 1;
    }
    // Double indirect blocks.
    else if (blk - LINN_INODE_DIR_BLOCKS < numPerBlock * numPerBlock)
    {
        depth = 2;
    }
    // Triple indirect blocks.
    else
    {
        depth = 3;
    }

    // Prepare read offset for the lookup
    offset  = inode->block[(LINN_INODE_DIR_BLOCKS + depth - 1)];
    offset *= super.blockSize;

    // Lookup the block number.
    while (true)
    {
        // Fetch block.
        if (storage->read(offset, block, super.blockSize) != FileSystem::Success)
        {
            return 0;
        }
        // Calculate the number of blocks remaining per entry.
        for (Size i = 0; i < depth - 1; i++)
        {
            remain *= LINN_SUPER_NUM_PTRS(&super);
        }
        // More indirection?
        if (remain == 1)
        {
            break;
        }
        // Calculate the next offset.
        offset  = block[ (blk - LINN_INODE_DIR_BLOCKS) / remain ];
        offset *= super.blockSize;
        remain  = 1;
        depth--;
    }

    // Calculate the final offset.
    const u32 offsetBlock = block[(blk - LINN_INODE_DIR_BLOCKS) % numPerBlock];

    // Calculate number of contiguous blocks following this block
    numContiguous = 1;

    for (Size i = blk + 1; i < numBlocks && (i % numPerBlock) != 0; i++)
    {
        if (block[(i - LINN_INODE_DIR_BLOCKS) % numPerBlock] == offsetBlock + numContiguous)
            numContiguous++;
        else
            break;
    }

    // All done.
    return offsetBlock * super.blockSize;
}

void LinnFileSystem::notSupportedHandler(FileSystemMessage *msg)
{
    msg->result = FileSystem::NotSupported;
    sendResponse(msg);
}
