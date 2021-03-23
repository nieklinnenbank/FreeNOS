/*
 * Copyright (C) 2015 Niek Linnenbank
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

#ifndef __LIB_LIBSTD_BYTEORDER_H
#define __LIB_LIBSTD_BYTEORDER_H

#include "Types.h"
#include "MemoryBlock.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libstd
 * @{
 */

#ifndef __BYTE_ORDER__
#error "__BYTE_ORDER__ not defined"
#endif /* __BYTE_ORDER__ */

#ifndef __ORDER_LITTLE_ENDIAN__
#error "__ORDER_LITTLE_ENDIAN__ not defined"
#endif /* __ORDER_LITTLE_ENDIAN__ */

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "This implementation only supports little-endian targets: define __BYTE_ORDER__ to __ORDER_LITTLE_ENDIAN__"
#endif

/**
 * Byte swap functions
 * @{
 */

/**
 * Byte swap a 16-bit integer
 *
 * @param x Integer input
 *
 * @return Byte-swapped integer value
 */
#define SWAP16(x) ((u16)(                   \
        (((u16)(x) & (u16)0x00ffU) << 8) |  \
        (((u16)(x) & (u16)0xff00U) >> 8)))

/**
 * Byte swap a 32-bit integer
 *
 * @param x Integer input
 *
 * @return Byte-swapped integer value
 */

#define SWAP32(x) ((u32)(                          \
        (((u32)(x) & (u32)0x000000ffUL) << 24) |   \
        (((u32)(x) & (u32)0x0000ff00UL) <<  8) |   \
        (((u32)(x) & (u32)0x00ff0000UL) >>  8) |   \
        (((u32)(x) & (u32)0xff000000UL) >> 24)))

/**
 * Byte swap a 64-bit integer
 *
 * @param x Integer input
 *
 * @return Byte-swapped integer value
 */

#define SWAP64(x) ((u64)(                          \
        (((u64)(x) & (u64)0x00000000000000ffUL) << 56) |   \
        (((u64)(x) & (u64)0x000000000000ff00UL) << 40) |   \
        (((u64)(x) & (u64)0x0000000000ff0000UL) << 24) |   \
        (((u64)(x) & (u64)0x00000000ff000000UL) <<  8) |   \
        (((u64)(x) & (u64)0x000000ff00000000UL) >>  8) |   \
        (((u64)(x) & (u64)0x0000ff0000000000UL) >> 24) |   \
        (((u64)(x) & (u64)0x00ff000000000000UL) >> 40) |   \
        (((u64)(x) & (u64)0xff00000000000000UL) >> 56)))

/**
 * @}
 */

/**
 * Integer conversion functions
 * @{
 */

/**
 * CPU byte order to little endian 64-bit.
 *
 * @param x Input integer.
 *
 * @return le64 type integer.
 */
#define cpu_to_le64(x) ((le64)(u64)(x))

/**
 * Little endian 64-bit to CPU byte order.
 *
 * @param x le64 type integer.
 *
 * @return CPU byte ordered 64-bit integer.
 */
#define le64_to_cpu(x) ((u64)(le64)(x))

/**
 * CPU byte order to little endian 32-bit.
 *
 * @param x Input integer.
 *
 * @return le32 type integer.
 */
#define cpu_to_le32(x) ((le32)(u32)(x))

/**
 * Little endian 32-bit to CPU byte order.
 *
 * @param x le32 type integer.
 *
 * @return CPU byte ordered 32-bit integer.
 */
#define le32_to_cpu(x) ((u32)(le32)(x))

/**
 * CPU byte order to little endian 16-bit.
 *
 * @param x Input integer.
 *
 * @return le16 type integer.
 */
#define cpu_to_le16(x) ((le16)(u16)(x))

/**
 * Little endian 16-bit to CPU byte order.
 *
 * @param x le16 type integer.
 *
 * @return CPU byte ordered 16-bit integer.
 */
#define le16_to_cpu(x) ((u16)(le16)(x))

/**
 * CPU byte order to big endian 64-bit.
 *
 * @param x Input integer.
 *
 * @return be64 type integer.
 */
#define cpu_to_be64(x) ((be64)SWAP64((x)))

/**
 * Big endian 64-bit to CPU byte order.
 *
 * @param x be64 type integer.
 *
 * @return CPU byte ordered 64-bit integer.
 */
#define be64_to_cpu(x) SWAP64((u64)(be64)(x))

/**
 * CPU byte order to big endian 32-bit.
 *
 * @param x Input integer.
 *
 * @return be32 type integer.
 */
#define cpu_to_be32(x) ((be32)SWAP32((x)))

/**
 * Big endian 32-bit to CPU byte order.
 *
 * @param x be32 type integer.
 *
 * @return CPU byte ordered 32-bit integer.
 */
#define be32_to_cpu(x) SWAP32((u32)(be32)(x))

/**
 * CPU byte order to big endian 16-bit.
 *
 * @param x Input integer.
 *
 * @return be16 type integer.
 */
#define cpu_to_be16(x) ((be16)SWAP16((x)))

/**
 * Big endian 16-bit to CPU byte order.
 *
 * @param x be16 type integer.
 *
 * @return CPU byte ordered 16-bit integer.
 */
#define be16_to_cpu(x) SWAP16((u16)(be16)(x))

/**
 * @}
 */

/**
 * Memory read/write functions
 *
 * @note These functions support unaligned memory pointers
 *
 * @{
 */

/**
 * Read 64-bit integer (no conversion)
 *
 * @param data Memory pointer, which can be unaligned.
 *
 * @return Unsigned 64-bit integer
 */
inline const u64 read64(const void *data)
{
    u64 value;
    MemoryBlock::copy(&value, data, sizeof(value));
    return value;
}

/**
 * Read 32-bit integer (no conversion)
 *
 * @param data Memory pointer, which can be unaligned.
 *
 * @return Unsigned 32-bit integer
 */
inline const u32 read32(const void *data)
{
    u32 value;
    MemoryBlock::copy(&value, data, sizeof(value));
    return value;
}

/**
 * Read 16-bit integer (no conversion)
 *
 * @param data Memory pointer, which can be unaligned.
 *
 * @return Unsigned 16-bit integer
 */
inline const u16 read16(const void *data)
{
    u16 value;
    MemoryBlock::copy(&value, data, sizeof(value));
    return value;
}

/**
 * Read 8-bit integer
 *
 * @param data Memory pointer, which can be unaligned.
 *
 * @return Unsigned 8-bit integer
 */
inline const u8 read8(const void *data)
{
    u8 value;
    MemoryBlock::copy(&value, data, sizeof(value));
    return value;
}

/**
 * Write 64-bit integer (no conversion)
 *
 * @param data Memory pointer to write to, which can be unaligned.
 * @param input Input integer
 */
inline void write64(void *data, const u64 input)
{
    MemoryBlock::copy(data, &input, sizeof(input));
}

/**
 * Write 32-bit integer (no conversion)
 *
 * @param data Memory pointer to write to, which can be unaligned.
 * @param input Input integer
 */
inline void write32(void *data, const u32 input)
{
    MemoryBlock::copy(data, &input, sizeof(input));
}

/**
 * Write 16-bit integer (no conversion)
 *
 * @param data Memory pointer to write to, which can be unaligned.
 * @param input Input integer
 */
inline void write16(void *data, const u16 input)
{
    MemoryBlock::copy(data, &input, sizeof(input));
}

/**
 * Write 8-bit integer
 *
 * @param data Memory pointer to write to, which can be unaligned.
 * @param input Input integer
 */
inline void write8(void *data, const u8 input)
{
    MemoryBlock::copy(data, &input, sizeof(input));
}

/**
 * Read 64-bit little endian integer
 *
 * @param data Memory pointer, which can be unaligned.
 *
 * @return Unsigned 64-bit integer
 */
inline const u64 readLe64(const void *data)
{
    u64 value;
    MemoryBlock::copy(&value, data, sizeof(value));
    return le64_to_cpu(value);
}

/**
 * Read 32-bit little endian integer
 *
 * @param data Memory pointer, which can be unaligned.
 *
 * @return Unsigned 32-bit integer
 */
inline const u32 readLe32(const void *data)
{
    u32 value;
    MemoryBlock::copy(&value, data, sizeof(value));
    return le32_to_cpu(value);
}

/**
 * Read 16-bit little endian integer
 *
 * @param data Memory pointer, which can be unaligned.
 *
 * @return Unsigned 16-bit integer
 */
inline const u16 readLe16(const void *data)
{
    u16 value;
    MemoryBlock::copy(&value, data, sizeof(value));
    return le16_to_cpu(value);
}

/**
 * Read 64-bit big endian integer
 *
 * @param data Memory pointer, which can be unaligned.
 *
 * @return Unsigned 64-bit integer
 */
inline const u64 readBe64(const void *data)
{
    u64 value;
    MemoryBlock::copy(&value, data, sizeof(value));
    return be64_to_cpu(value);
}

/**
 * Read 32-bit big endian integer
 *
 * @param data Memory pointer, which can be unaligned.
 *
 * @return Unsigned 32-bit integer
 */
inline const u32 readBe32(const void *data)
{
    u32 value;
    MemoryBlock::copy(&value, data, sizeof(value));
    return be32_to_cpu(value);
}

/**
 * Read 16-bit big endian integer
 *
 * @param data Memory pointer, which can be unaligned.
 *
 * @return Unsigned 16-bit integer
 */
inline const u16 readBe16(const void *data)
{
    u16 value;
    MemoryBlock::copy(&value, data, sizeof(value));
    return be16_to_cpu(value);
}

/**
 * Write 64-bit little endian integer
 *
 * @param data Memory pointer to write to, which can be unaligned.
 * @param input Input integer
 */
inline void writeLe64(void *data, const u64 input)
{
    const u64 value = cpu_to_le64(input);
    MemoryBlock::copy(data, &value, sizeof(value));
}

/**
 * Write 32-bit little endian integer
 *
 * @param data Memory pointer to write to, which can be unaligned.
 * @param input Input integer
 */
inline void writeLe32(void *data, const u32 input)
{
    const u32 value = cpu_to_le32(input);
    MemoryBlock::copy(data, &value, sizeof(value));
}

/**
 * Write 16-bit little endian integer
 *
 * @param data Memory pointer to write to, which can be unaligned.
 * @param input Input integer
 */
inline void writeLe16(void *data, const u16 input)
{
    const u16 value = cpu_to_le16(input);
    MemoryBlock::copy(data, &value, sizeof(value));
}

/**
 * Write 64-bit big endian integer
 *
 * @param data Memory pointer to write to, which can be unaligned.
 * @param input Input integer
 */
inline void writeBe64(void *data, const u64 input)
{
    const u64 value = cpu_to_be64(input);
    MemoryBlock::copy(data, &value, sizeof(value));
}

/**
 * Write 32-bit big endian integer
 *
 * @param data Memory pointer to write to, which can be unaligned.
 * @param input Input integer
 */
inline void writeBe32(void *data, const u32 input)
{
    const u32 value = cpu_to_be32(input);
    MemoryBlock::copy(data, &value, sizeof(value));
}

/**
 * Write 16-bit big endian integer
 *
 * @param data Memory pointer to write to, which can be unaligned.
 * @param input Input integer
 */
inline void writeBe16(void *data, const u16 input)
{
    const u16 value = cpu_to_be16(input);
    MemoryBlock::copy(data, &value, sizeof(value));
}

/**
 * @}
 */

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBSTD_BYTEORDER_H */
