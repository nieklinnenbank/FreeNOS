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

#include <FreeNOS/User.h>
#include "LinnFileSystem.h"
#include "LinnFile.h"

LinnFile::LinnFile(LinnFileSystem *f, LinnInode *i)
    : fs(f), inode(i)
{
    m_size   = inode->size;
    m_access = inode->mode;
}

LinnFile::~LinnFile()
{
}

FileSystem::Error LinnFile::read(IOBuffer & buffer, Size size, Size offset)
{
    LinnSuperBlock *sb;
    Size bytes = 0, blockNr = 0;
    u64 storageOffset, copyOffset = offset;
    static u8 block[LINN_MAX_BLOCK_SIZE];
    Size total = 0;
    FileSystem::Error e;

    // Initialize variables.
    sb     = fs->getSuperBlock();
    assert(sb->blockSize <= LINN_MAX_BLOCK_SIZE);

    // Skip ahead blocks.
    while ((sb->blockSize * (blockNr + 1)) <= copyOffset)
    {
        blockNr++;
    }
    // Adjust the copy offset within this block.
    copyOffset -= sb->blockSize * blockNr;

    // Loop all blocks.
    while (blockNr < LINN_INODE_NUM_BLOCKS(sb, inode) &&
           total < size && inode->size - (offset + total) > 0)
    {
        // Calculate the offset in storage for this block.
        storageOffset = fs->getOffset(inode, blockNr);

        // Fetch the next block.
        if (fs->getStorage()->read(storageOffset, block, sb->blockSize) < 0)
        {
            return FileSystem::IOError;
        }

        // Calculate the number of bytes to copy.
        bytes = sb->blockSize - copyOffset;

        // Respect the inode size.
        if (bytes > inode->size - (offset + total))
        {
            bytes = inode->size - (offset + total);
        }

        // Respect the remote process buffer.
        if (bytes > size - total)
        {
            bytes = size - total;
        }

        // Copy into the buffer.
        if ((e = buffer.write(block + copyOffset, bytes, total)) < 0)
        {
            return e;
        }

        // Update state.
        total      += bytes;
        copyOffset  = 0;
        blockNr++;
    }
    // Success.
    return (Error) total;
}
