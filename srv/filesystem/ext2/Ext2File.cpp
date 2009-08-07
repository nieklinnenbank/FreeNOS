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

#include <API/VMCopy.h>
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

Error Ext2File::read(IOBuffer *buffer, Size size, Size offset)
{
    Ext2SuperBlock *sb = ext2->getSuperBlock();
    u8 *block = new u8[EXT2_BLOCK_SIZE(sb)];
    Size bytes = 0, total = 0, blockNr = 0;
    Error e = ESUCCESS;
    u64 storageOffset, copyOffset = offset;

    /* Skip ahead blocks. */
    while ((EXT2_BLOCK_SIZE(sb) * (blockNr + 1)) <= copyOffset)
    {
	blockNr++;
    }
    /* Adjust the copy offset within this block. */
    copyOffset -= EXT2_BLOCK_SIZE(sb) * blockNr;

    /* Loop all blocks. */
    for (; blockNr < inode->blocks - 1 && total < size &&
	   inode->size - (offset + total); blockNr++)
    {
	/* Calculate the offset in storage for this block. */
	storageOffset = ext2->getOffset(inode, blockNr);

        /* Fetch the next block. */
        if (ext2->getStorage()->read(storageOffset, block,
				     EXT2_BLOCK_SIZE(sb)) < 0)
	{
	    e = EACCES;
	    break;
	}
	/* Calculate the number of bytes to copy. */
	bytes = EXT2_BLOCK_SIZE(sb) - copyOffset;
	
	/* Respect the inode size. */
	if (bytes > inode->size - (offset + total))
	{
	    bytes = inode->size - (offset + total);
	}
	/* Respect the remote process buffer. */
	if (bytes > size - total)
	{
	    bytes = size - total;
	}
        /* Copy to the output buffer. */
	if ((e = buffer->write(block + copyOffset, bytes, total)) < 0)
	{
	    return e;
	}
	/* Update state. */
	buffer     += bytes;
	total      += bytes;
	copyOffset  = 0;
	e           = ESUCCESS;
     }
    /* Success. */
    delete block;
    return total;
}
