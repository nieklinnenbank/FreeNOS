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

#ifndef __BYTEORDER_H
#define __BYTEORDER_H

#include <Arch/CPU.h>
#include <Macros.h>

#ifdef CPU_BIG_ENDIAN

#elif CPU_LITTLE_ENDIAN

#define FORCE

/**
 * CPU byte order to little endian 64-bit.
 * @param x Input integer.
 * @return le64 type integer.
 */
#define cpu_to_le64(x) ((FORCE le64)(u64)(x))

/**
 * Little endian 64-bit to CPU byte order.
 * @param x le64 type integer.
 * @return CPU byte ordered 64-bit integer.
 */
#define le64_to_cpu(x) ((FORCE u64)(le64)(x))

/**
 * CPU byte order to little endian 32-bit.
 * @param x Input integer.
 * @return le32 type integer.
 */
#define cpu_to_le32(x) ((FORCE le32)(u32)(x))

/**
 * Little endian 32-bit to CPU byte order.
 * @param x le32 type integer.
 * @return CPU byte ordered 32-bit integer.
 */
#define le32_to_cpu(x) ((FORCE u32)(le32)(x))

/**
 * CPU byte order to little endian 16-bit.
 * @param x Input integer.
 * @return le16 type integer.
 */
#define cpu_to_le16(x) ((FORCE le16)(u16)(x))

/**
 * Little endian 16-bit to CPU byte order.
 * @param x le16 type integer.
 * @return CPU byte ordered 16-bit integer.
 */
#define le16_to_cpu(x) ((FORCE u16)(le16)(x))

/**
 * CPU byte order to big endian 64-bit.
 * @param x Input integer.
 * @return be64 type integer.
 */
#define cpu_to_be64(x) ((FORCE be64)swab64((x)))

/**
 * Big endian 64-bit to CPU byte order.
 * @param x be64 type integer.
 * @return CPU byte ordered 64-bit integer.
 */
#define be64_to_cpu(x) swab64((FORCE u64)(be64)(x))

/**
 * CPU byte order to big endian 32-bit.
 * @param x Input integer.
 * @return be32 type integer.
 */
#define cpu_to_be32(x) ((FORCE be32)swab32((x)))

/**
 * Big endian 32-bit to CPU byte order.
 * @param x be32 type integer.
 * @return CPU byte ordered 32-bit integer.
 */
#define be32_to_cpu(x) swab32((FORCE u32)(be32)(x))

/**
 * CPU byte order to big endian 16-bit.
 * @param x Input integer.
 * @return be16 type integer.
 */
#define cpu_to_be16(x) ((FORCE be16)swab16((x)))

/**
 * Big endian 16-bit to CPU byte order.
 * @param x be16 type integer.
 * @return CPU byte ordered 16-bit integer.
 */
#define be16_to_cpu(x) swab16((FORCE u16)(be16)(x))

#endif /* LITTLE_ENDIAN */

/**
 * @}
 */

#endif /* BYTEORDER_H */
