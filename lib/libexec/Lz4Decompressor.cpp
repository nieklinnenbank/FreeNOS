/*
 * Copyright (C) 2020 Niek Linnenbank
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
#include <Log.h>
#include <ByteOrder.h>
#include <MemoryBlock.h>
#include "Lz4Decompressor.h"

Lz4Decompressor::Lz4Decompressor(const void *data, const Size size)
    : m_inputData(static_cast<const u8 *>(data))
    , m_inputSize(size)
    , m_frameDescSize(3)
    , m_blockChecksums(false)
    , m_contentChecksum(false)
    , m_contentSize(0)
    , m_blockMaximumSize(0)
{
}

Lz4Decompressor::Result Lz4Decompressor::initialize()
{
    // Reset state
    m_frameDescSize = 3;

    // Verify minimum input size
    if (m_inputSize < 27)
    {
        ERROR("invalid size of input data: " << m_inputSize);
        return InvalidArgument;
    }

    // Verify the input is an actual LZ4 frame
    if (readLe32(m_inputData) != FrameMagic)
    {
        ERROR("invalid magic value " << readLe32(m_inputData) << " != " << FrameMagic);
        return InvalidArgument;
    }

    // Read the FLG byte
    const u8 flg = *(m_inputData + sizeof(u32));

    // Verify the version bits
    const u8 version = flg >> FrameVersionShift;
    if (version != FrameVersion)
    {
        ERROR("invalid version value " << version << " != " << FrameVersion);
        return InvalidArgument;
    }

    // This code only supports independent blocks
    const bool independent = (flg >> FrameBlockIndShift) & 0x1;
    if (!independent)
    {
        ERROR("inter-dependent blocks not supported");
        return NotSupported;
    }

    // Check for block checksum flag
    m_blockChecksums = (flg >> FrameBlockChkShift) & 0x1;

    // Check for content size flag
    if ((flg >> FrameContentSzShift) & 0x1)
    {
        m_contentSize = readLe64(m_inputData + sizeof(u32) + (sizeof(u8) * 2));
        m_frameDescSize += 8;
    }

    // Content size must be non-zero
    if (m_contentSize == 0)
    {
        ERROR("content size must not be zero");
        return NotSupported;
    }

    // Check for the content checksum flag
    m_contentChecksum = (flg >> FrameContentChkShift) & 0x1 ? true : false;

    // Check for the DictID flag
    if ((flg >> FrameDictIdShift) & 0x1)
    {
        m_frameDescSize += 4;
    }

    // Read the BD byte which contains the maximum block size
    const u8 bd = *(m_inputData + sizeof(u32) + sizeof(u8));
    switch (bd >> 4)
    {
        case 4:
            m_blockMaximumSize = KiloByte(64);
            break;
        case 5:
            m_blockMaximumSize = KiloByte(256);
            break;
        case 6:
            m_blockMaximumSize = MegaByte(1);
            break;
        case 7:
            m_blockMaximumSize = MegaByte(4);
            break;
        default:
        {
            ERROR("invalid maximum block size value: " << (bd >> 4));
            return InvalidArgument;
        }
    }

    return Success;
}

u64 Lz4Decompressor::getUncompressedSize() const
{
    return m_contentSize;
}

Lz4Decompressor::Result Lz4Decompressor::read(void *buffer,
                                              const Size size) const
{
    const u8 *input = m_inputData + m_frameDescSize + sizeof(u32);
    const u8 *inputEnd = m_inputData + m_inputSize;
    u8 *output = static_cast<u8 *>(buffer);
    Size copied = 0;

    while (copied < size && input < inputEnd)
    {
        // Fetch the next block
        const u32 blockSizeByte = readLe32(input);
        const u32 blockSize = blockSizeByte & ~(1 << 31);
        const bool isCompressed = blockSizeByte & (1 << 31) ? false : true;
        Size uncompSize;

        // Last block has the EndMark as size value
        if (blockSize == EndMark)
        {
            break;
        }
        assert(blockSize <= m_blockMaximumSize);
        input += sizeof(u32);

        // Decompress the block
        if (isCompressed)
        {
            uncompSize = decompress(input, blockSize, output, size - copied);
        }
        // Return data as-is when the block is not compressed
        else
        {
            MemoryBlock::copy(output, input, blockSize);
            uncompSize = blockSize;
        }

        // Move to the next block
        copied += uncompSize;
        output += uncompSize;
        input += blockSize;
        if (m_blockChecksums)
        {
            input += sizeof(u32);
        }
    }

    return Success;
}

inline const u32 Lz4Decompressor::integerDecode(const u32 initial,
                                                const u8 *next,
                                                Size &byteCount) const
{
    u32 value = initial;

    if (initial < 0xf)
    {
        return initial;
    }

    for (byteCount = 1; ; byteCount++)
    {
        const u8 byte = *next++;
        value += byte;

        if (byte < 0xff)
        {
            break;
        }
    }

    return value;
}

const u32 Lz4Decompressor::decompress(const u8 *input,
                                      const Size inputSize,
                                      u8 *output,
                                      const Size outputSize) const
{
    const u8 *inputEnd = input + inputSize;
    Size outputOffset = 0;

    // Decompress the whole block
    while (input < inputEnd && outputOffset < outputSize)
    {
        u32 literalBytes = 0;
        u32 matchBytes = 0;

        // Read the token
        const u8 token = *input;
        input++;

        // Read literals count
        const u32 literalsCount = integerDecode(token >> 4, input, literalBytes);
        input += literalBytes;
        DEBUG("token = " << token << " literalsCount = " << literalsCount << " literalBytes = " << literalBytes);

        // Copy literals
        if (literalsCount > 0)
        {
            MemoryBlock::copy(output + outputOffset, input, literalsCount);
            input += literalsCount;
            outputOffset += literalsCount;
        }

        // End of block reached? Last 5 bytes are only literals
        if (input >= inputEnd)
        {
            break;
        }

        // Read match offset
        const u16 off = readLe16(input);
        assert(off <= outputOffset);
        const u32 matchOffset = outputOffset - off;
        input += sizeof(u16);

        // Read match length
        const u32 matchCount = integerDecode(token & 0xf, input, matchBytes) + 4u;
        input += matchBytes;

        // Copy the match from previous decoded bytes
        DEBUG("matchOffset = " << matchOffset << " matchCount = " << matchCount);
        MemoryBlock::copy(output + outputOffset, output + matchOffset, matchCount);
        outputOffset += matchCount;
    }

    assert(outputOffset <= m_blockMaximumSize);
    return outputOffset;
}
