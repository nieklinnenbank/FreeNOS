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
#include <Directory.h>
#include <Device.h>
#include <LogMessage.h>
#include <stdlib.h>
#include <string.h>
#include "Ext2FileSystem.h"
#include "Ext2SuperBlock.h"
#include "Ext2File.h"
#include "Ext2Directory.h"
#include "Ext2Inode.h"
#include "Ext2Group.h"

Ext2FileSystem::Ext2FileSystem(const char *p, Storage *s)
    : FileSystem(p), storage(s), groups(ZERO)
{
    FileSystemPath slash("/");
    Ext2Inode *rootInode;
    Ext2Group *group;
    Size offset;
    Error e;

    /* Read out the superblock. */
    if ((e = s->read(EXT2_SUPER_OFFSET, (u8 *) &superBlock,
		     sizeof(superBlock))) <= 0)
    {
	log("Ext2: reading superblock failed: %s",
	     strerror(e));
	exit(EXIT_FAILURE);
    }
    /* Verify magic. */
    if (superBlock.magic != EXT2_SUPER_MAGIC)
    {
	log("Ext2: %x != EXT2_SUPER_MAGIC",
	     superBlock.magic);
	exit(EXIT_FAILURE);
    }
    /* Create groups vector. */
    groups  = new Vector<Ext2Group>(EXT2_GROUPS_COUNT(&superBlock));

    /* Read out group descriptors. */
    for (Size i = 0; i < EXT2_GROUPS_COUNT(&superBlock); i++)
    {
	/* Allocate buffer. */
	group   = new Ext2Group;
	offset  = EXT2_SUPER_OFFSET;
	offset += le32_to_cpu(superBlock.firstDataBlock) *
	          EXT2_BLOCK_SIZE(&superBlock);
	offset += sizeof(Ext2Group) * i;

	/* Read from storage. */
	if ((e = s->read(offset, (u8 *) group, sizeof(Ext2Group))) <= 0)
	{
	    log("Ext2: reading group descriptor failed: %s",
		 strerror(e));
	    exit(EXIT_FAILURE);
	}
	/* Insert in the groups vector. */
	groups->insert(i, group);
    }
    log("Ext2: %d group descriptors",
	 EXT2_GROUPS_COUNT(&superBlock));
    
    /* Debug out superblock information. */
    log("Ext2: %d inodes, %d blocks",
	 superBlock.inodesCount, superBlock.blocksCount);

    /* Read out the root directory. */
    rootInode = getInode(EXT2_ROOT_INO);
    root = new FileCache(&slash, new Ext2Directory(this, rootInode), ZERO);
    log("Ext2: mounted '%s'", p);
}

Error Ext2FileSystem::createFile(FileSystemMessage *msg,
				 FileSystemPath *path)
{
    return ENOSUPPORT;
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
    if ((e = storage->read(offset, (u8 *) inode, sizeof(Ext2Inode))) <= 0)
    {
        log("Ext2: reading inode failed: %s",
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
    Size depth = 0;
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
	if (storage->read(offset, (u8 *) block,
			  EXT2_BLOCK_SIZE(&superBlock)) < 0)
	{
	    delete block;
	    return 0;
	}
	/* Calculate the number of blocks remaining per entry. */
	Size remain = EXT2_ADDR_PER_BLOCK(&superBlock);
	
	/* Effectively the pow() function. */
	for (Size i = 0; i < depth - 1; i++)
	{
	    remain *= remain;
	}
	/* Calculate the next offset. */
	offset  = block[ remain / (blk - EXT2_NDIR_BLOCKS + 1) ];
	offset *= EXT2_BLOCK_SIZE(&superBlock);
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

FileCache * Ext2FileSystem::lookupFile(FileSystemPath *path)
{
    List<String> *entries = path->split();
    FileCache *c = root;
    Ext2Inode *inode;
    Ext2DirectoryEntry entry;
    Ext2Directory *dir;

    /* Loop the entire path. */
    for (ListIterator<String> i(entries); i.hasNext(); i++)
    {
	/* Do we have this entry? */
        if (!c->entries[i.current()])
	{
	    /* If this isn't a directory, we cannot perform a lookup. */
	    if (c->file->getType() != DirectoryFile)
	    {
		return ZERO;
	    }
	    /* Then retrieve it, if possible. */	
	    dir = (Ext2Directory *) c->file;
	    if (dir->getEntry(&entry, **i.current()) != ESUCCESS)
	    {
		return ZERO;
	    }
	    /* Lookup corresponding inode. */
	    if (!(inode = getInode(entry.inode)))
	    {
		return ZERO;
	    }
	    /* Create the appropriate in-memory file. */
	    switch (EXT2_FILETYPE(inode))
	    {
	        case DT_DIR:
		    c = insertFileCache(new Ext2Directory(this, inode),
		    			**i.current());
		    break;
			
		//case DT_DEV:
		//    c = insertFileCache(new Ext2Device(this, inode),
		//			i.current());
		//    break;
			
		//case DT_FIFO:
		//case DT_SYM:
		//case DT_LINK:
		//case DT_SOCK:
		case DT_REG:
		    c = insertFileCache(new Ext2File(this, inode),
					**i.current());
		    break;

		case DT_UNKNOWN:
		default:
		    return ZERO;
	    }
	}
	/* Move to the next entry. */
	else
	    c = c->entries[i.current()];
    }
    return c;
}
