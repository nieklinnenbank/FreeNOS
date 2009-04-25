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

#include <api/VMCopy.h>
#include <string.h>
#include "Ext2File.h"

Ext2File::Ext2File(Ext2FileSystem *fs, Ext2Inode *i)
    : ext2(fs), inode(i)
{
    size = inode->size;
}

Ext2File::~Ext2File()
{
}

Error Ext2File::read(FileSystemMessage *msg)
{
    Ext2SuperBlock *sb = ext2->getSuperBlock();
    u8 *block = new u8[EXT2_BLOCK_SIZE(sb)], *buffer = (u8 *) msg->buffer;
    Size bytes = 0, total = 0;
    Error e = ESUCCESS;
    u64 offset;

    /* Loop all blocks. */
    for (Size blk = 0; blk < inode->blocks - 1 && total < msg->size; blk++)
    {
	/* Skip ahead blocks. */
	if (EXT2_BLOCK_SIZE(sb) * blk < msg->offset)
	{
	    continue;
	}
	/* Calculate the offset in storage for this block. */
	offset = ext2->getOffset(inode, blk);

        /* Fetch the next block. */
        if (ext2->getStorage()->read(offset, block,
				     EXT2_BLOCK_SIZE(sb)) < 0)
	{
	    e = EACCESS;
	    break;
	}
	/* Calculate the number of bytes to copy. */
	bytes = EXT2_BLOCK_SIZE(sb) < (msg->size - total) ?
	        EXT2_BLOCK_SIZE(sb) : (msg->size - total);
	
        /* Copy to the remote process. */
        if ((e = VMCopy(msg->procID, Write, (Address) block,
                       (Address) buffer, bytes)) < 0)
        {
	    break;
	}
	buffer += EXT2_BLOCK_SIZE(sb);
	total  += bytes;
	e       = ESUCCESS;
    }
    msg->size = total;
    delete block;
    return e;
}

Error Ext2File::write(FileSystemMessage *msg)
{
    return ENOSUPPORT;
}
