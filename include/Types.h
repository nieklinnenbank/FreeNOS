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

#ifndef __TYPES_H
#define __TYPES_H

/** Unsigned integer. */
typedef unsigned int uint;

/** Unsigned long integer. */
typedef unsigned long ulong;

/** Signed integer. */
typedef signed int sint;

/** Signed long integer. */
typedef signed long slong;

/** Unsigned 64 bits. */
typedef unsigned long long u64;

/** Unsigned 32 bits. */
typedef unsigned int u32;

/** Unsigned 16 bits. */
typedef unsigned short u16;

/** Unsigned 8 bits. */
typedef unsigned char u8;

/** Signed 64 bits. */
typedef signed long long s64;

/** Signed 32 bits. */
typedef signed int s32;

/** Signed 16 bits. */
typedef signed short int s16;

/** Signed 8 bits. */
typedef signed char s8;

/** Any sane size indicator cannot go negative. */
typedef unsigned int Size;

/** A memory address. */
typedef unsigned long Address;

/**
 * Error code defined in Error.h
 * @see Error.h
 */
typedef slong Error;

#endif /* __TYPES_H */
