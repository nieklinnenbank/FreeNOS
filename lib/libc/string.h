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

#ifndef __LIBC_STRING_H
#define __LIBC_STRING_H

#include <sys/types.h>
#include "errno.h"

/**
 * @defgroup ansic ISO/IEC 9899:1999
 * @{
 */

/**
 * Compare two strings.
 * @param dest Destination string to compare.
 * @param src Source string to compare.
 * @return Zero if equal, non-zero if not.
 */
extern C int strcmp(char *dest, char *src);

/**
 * Compare two strings, by only a maximum number of bytes.
 * @param dest Destination string to compare.
 * @param src Source string to compare.
 * @param count Maximum number of bytes to compare.
 * @return Zero if equal, non-zero if not.
 */
extern C int strncmp(char *dest, char *src, size_t count);

/**
 * Fill memory with a constant byte.
 * @param dest Memory to write to.
 * @param ch Constant byte.
 * @return Pointer to dest.
 */
extern C void * memset(void *dest, int ch, size_t count);

/**
 * Copy memory from one place to another.
 * @param dest Destination address.
 * @param src Source address.
 * @param count Number of bytes to copy.
 * @return The destination address.
 */
extern C void * memcpy(void *dest, const void *src, size_t count);

/**
 * Calculate the length of a string.
 * @param str String to calculate length for.
 * @return Length of the string.
 */
extern C int strlen(char *str);

/**
 * Copy a string.
 * @param dest Destination buffer.
 * @param src Source string.
 * @return Number of bytes copied.
 */
extern C int strcpy(char *dest, char *src);

/**
 * Copy a string, given a maximum number of bytes.
 * @param dest Destination buffer.
 * @param src Source string.
 * @param sz Maximum number of bytes to copy.
 * @return Number of bytes copied.
 */
extern C int strncpy(char *dest, char *src, size_t sz);

/**
 * Copy src to string dst of size siz. At most siz-1 characters 
 * will be copied.  Always NUL terminates (unless siz == 0).
 * @note This function is copied from OpenBSD-4.3
 * @param dst Destination string
 * @param src Source string
 * @param siz size_t of dst buffer
 * @return strlen(src); if retval >= siz, truncation occurred. 
 */
extern C size_t strlcpy(char *dst, const char *src, size_t siz);

/**
 * Append strings.
 * @param dest Destination string.
 * @param src String to be appended to the dest.
 * @return Pointer to the resulting string.
 */
extern C char * strcat(char *dest, char *src);

/**
 * The strerror function maps the number in errnum to a message string.
 * @param errnum Error number to convert.
 * @return The strerror function returns a pointer to the string, the contents
 *         of which are locale specific. The array pointed to shall not be modified
 *         by the program, but may be overwritten by a subsequent call to the
 *         strerror function.
 */
extern C char * strerror(int errnum);

/**
 * @}
 */

#endif /* __LIBC_STRING_H */
