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

#ifndef __LIB_LIBEXEC_LZ4DECOMPRESSOR_H
#define __LIB_LIBEXEC_LZ4DECOMPRESSOR_H

#include <Types.h>
#include <Index.h>

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libexec
 * @{
 */

/**
 * Decompress data using the LZ4 algorithm created by Yann Collet.
 *
 * @see http://www.lz4.org
 * @see https://lz4.github.io/lz4/
 * @see https://en.wikipedia.org/wiki/LZ4_(compression_algorithm)
 */
class Lz4Decompressor
{
  private:

    /** Magic number value marks the start of the frame header */
    static const u32 FrameMagic = 0x184D2204;

    /**
     * Frame FLG Field Shift values
     */
    enum FrameFlgFields
    {
        FrameDictIdShift     = 0,
        FrameContentChkShift = 2,
        FrameContentSzShift  = 3,
        FrameBlockChkShift   = 4,
        FrameBlockIndShift   = 5,
        FrameVersionShift    = 6,
    };

    /** Current supported version of the LZ4 algorithm */
    static const u8 FrameVersion = 0x01;

    /** EndMark marks the end of the data stream */
    static const u32 EndMark = 0x00000000;

  public:

    /**
     * Result codes
     */
    enum Result
    {
        Success,
        IOError,
        InvalidArgument,
        NotSupported
    };

  public:

    /**
     * Constructor function.
     *
     * @param data Input data buffer
     * @param size Size in bytes of the input buffer
     */
    Lz4Decompressor(const void *data, const Size size);

    /**
     * Initialize the decompressor.
     *
     * @return Result code
     */
    Result initialize();

    /**
     * Get size of the uncompressed data
     *
     * @return Total size in bytes
     */
    u64 getUncompressedSize() const;

    /**
     * Reads compressed data.
     *
     * @param buffer Output buffer.
     * @param size Number of bytes of uncompressed data to copy.
     *
     * @return Result code
     */
    Result read(void *buffer, const Size size) const;

  private:

    /**
     * Decompress a block of compressed data
     *
     * @param input Compressed block data
     * @param inputSize Number of compressed bytes
     * @param output Output buffer to decompress to
     * @param outputSize Maximum number of bytes to decompress
     *
     * @return Number of bytes decompressed
     */
    const u32 decompress(const u8 *input,
                         const Size inputSize,
                         u8 *output,
                         const Size outputSize) const;

    /**
     * Decode input data as integer (little-endian, 32-bit unsigned)
     *
     * @param initial The initial integer value to use
     * @param next Pointer to next data to decode as integer
     * @param byteCount On output contains the number of bytes decoded
     *
     * @return Decoded unsigned 32-bit integer value
     */
    inline const u32 integerDecode(const u32 initial,
                                   const u8 *next,
                                   Size &byteCount) const;

  private:

    /** Compressed input data */
    const u8 *m_inputData;

    /** Total size in bytes of the compressed input data */
    const Size m_inputSize;

    /** Size of the frame descriptor in bytes */
    Size m_frameDescSize;

    /** True if blocks have checksums enabled */
    bool m_blockChecksums;

    /** True if the input data buffer contains content checksum */
    bool m_contentChecksum;

    /** Content size as specified in the frame header */
    u64 m_contentSize;

    /** Maximum block size in bytes of the uncompressed content */
    Size m_blockMaximumSize;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBEXEC_LZ4DECOMPRESSOR_H */
