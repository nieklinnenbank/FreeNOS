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

LinnFile::LinnFile(LinnFileSystem *fs,
                   const u32 inode,
                   LinnInode *inodeData)
    : File(inode)
    , m_fs(fs)
    , m_inodeData(inodeData)
{
    m_size   = m_inodeData->size;
    m_access = m_inodeData->mode;
}

LinnFile::~LinnFile()
{
}

FileSystem::Result LinnFile::read(IOBuffer & buffer,
                                  Size & size,
                                  const Size offset)
{
    const LinnSuperBlock *sb = m_fs->getSuperBlock();
    const Size inodeNumBlocks = LINN_INODE_NUM_BLOCKS(sb, m_inodeData);
    Size bytes = 0, blockNr = 0, blockCount;
    u64 storageOffset, copyOffset = offset;
    Size total = 0;

    assert(sb->blockSize <= LINN_MAX_BLOCK_SIZE);

    // Skip ahead blocks.
    while ((sb->blockSize * (blockNr + 1)) <= copyOffset)
    {
        blockNr++;
    }

    // Adjust the copy offset within this block.
    copyOffset -= sb->blockSize * blockNr;

    // Loop all blocks.
    while (blockNr < inodeNumBlocks && total < size && m_inodeData->size - (offset + total) > 0)
    {
        // Calculate the offset in storage for this block.
        storageOffset = m_fs->getOffsetRange(m_inodeData, blockNr, blockCount);

        // Calculate the number of bytes to copy.
        bytes = (blockCount * sb->blockSize) - copyOffset;

        // Respect the inode size.
        if (bytes > m_inodeData->size - (offset + total))
        {
            bytes = m_inodeData->size - (offset + total);
        }

        // Respect the remote process buffer.
        if (bytes > size - total)
        {
            bytes = size - total;
        }

        // Fetch the next block.
        if (m_fs->getStorage()->read(storageOffset + copyOffset,
                                     buffer.getBuffer() + total, bytes) != FileSystem::Success)
        {
            return FileSystem::IOError;
        }
        buffer.addCount(bytes);

        // Update state.
        total += bytes;
        copyOffset = 0;
        blockNr += blockCount;
    }

    // Success.
    size = total;
    return FileSystem::Success;
}
