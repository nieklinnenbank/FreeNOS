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

#ifndef __LIBC_STDIO_H
#define __LIBC_STDIO_H

#include <stdarg.h>
#include <Macros.h>

/**
 * @defgroup ansic ISO/IEC 9899:1999
 * @{
 */

/**
 * Output a debug message using printf().
 * @param fmt Formatted string.
 * @param ... Argument list.
 * @see printf
 */
#define dprintf(fmt, ...) \
    printf("{%s:%d}: " fmt, __FILE__, __LINE__, ##__VA_ARGS__);

/**
 * Write a formatted string into a buffer.
 * @param buffer String buffer to write to.
 * @param size Maximum number of bytes to write.
 * @param fmt Formatted string.
 * @param ... Argument list.
 * @return Number of bytes written to the buffer.
 */
extern C int snprintf(char *buffer, unsigned int size, const char *fmt, ...);

/**
 * Write a formatted string into a buffer.
 * @param buffer String buffer to write to.
 * @param size Maximum number of bytes to write.
 * @param fmt Formatted string.
 * @param args Argument list.
 * @return Number of bytes written to the buffer.
 */
extern C int vsnprintf(char *buffer, unsigned int size, const char *fmt, va_list args);

/**
 * Output a formatted string to standard output.
 * @param format Formatted string.
 * @param ... Argument list.
 * @return Number of bytes written or error code on failure.
 */
extern C int printf(char *format, ...);

/**
 * Output a formatted string to standard output, using a variable argument list.
 * @param format Formatted string.
 * @param ... Argument list.
 * @return Number of bytes written or error code on failure.
 */
extern C int vprintf(char *format, va_list args);

/**
 * @}
 */

#endif /* __LIBC_STDIO_H */
