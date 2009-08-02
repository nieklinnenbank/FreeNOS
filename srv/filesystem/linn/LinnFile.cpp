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
#include "LinnFile.h"
#include <string.h>

LinnFile::LinnFile(LinnFileSystem *f, LinnInode *i)
    : fs(f), inode(i)
{
    size   = inode->size;
    access = inode->mode;
}

LinnFile::~LinnFile()
{
}

Error LinnFile::read(FileSystemMessage *msg)
{
    LinnSuperBlock *sb;
    u8 *block, *buffer;
    Size bytes = 0, total = 0, blockNr = 0;
    u64 storageOffset, copyOffset = msg->offset;
    Error e = ESUCCESS;

    /* Initialize variables. */
    sb     = fs->getSuperBlock();
    block  = new u8[sb->blockSize];
    buffer = (u8 *) msg->buffer;

    /* Skip ahead blocks. */
    while ((sb->blockSize * (blockNr + 1)) <= copyOffset)
    {
	blockNr++;
    }
    /* Adjust the copy offset within this block. */
    copyOffset -= sb->blockSize * blockNr;

    /* Loop all blocks. */
    while (blockNr < LINN_INODE_NUM_BLOCKS(sb, inode) &&
	   total   < msg->size &&
	   inode->size - (msg->offset + total) > 0)
    {
	/* Calculate the offset in storage for this block. */
	storageOffset = fs->getOffset(inode, blockNr);

        /* Fetch the next block. */
        if (fs->getStorage()->read(storageOffset, block, sb->blockSize) < 0)
	{
	    e = EACCES;
	    break;
	}
	/* Calculate the number of bytes to copy. */
	bytes = sb->blockSize - copyOffset;
	
	/* Respect the inode size. */
	if (bytes > inode->size - (msg->offset + total))
	{
	    bytes = inode->size - (msg->offset + total);
	}
	/* Respect the remote process buffer. */
	if (bytes > msg->size - total)
	{
	    bytes = msg->size - total;
	}
        /* Copy to the remote process. */
        if ((e = VMCopy(msg->from, Write,
	               ((Address) (block)) + copyOffset,
                        (Address) (buffer), bytes)) < 0)
        {
	    break;
	}
	/* Update state. */
	buffer     += bytes;
	total      += bytes;
	copyOffset  = 0;
	e           = ESUCCESS;
	blockNr++;
    }
    /* Success. */
    msg->size = total;
    delete block;
    return e;
}

Error LinnFile::write(FileSystemMessage *msg)
{
    return ENOTSUP;
}
