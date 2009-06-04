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

#ifndef __LIBPOSIX_UNISTD_H
#define __LIBPOSIX_UNISTD_H

#include <Macros.h>
#include <stdio.h>
#include "types.h"

/**                                                                                                                                                                                                     
 * @defgroup libposix libposix (POSIX.1-2008)
 * @{
 */ 

/**
 * Integer value indicating version of this standard (C-language binding)
 * to which the implementation conforms. For implementations conforming to
 * POSIX.1-2008, the value shall be 200809L.
 */
#define _POSIX_VERSION 200809L

/**
 * Integer value indicating version of the Shell and Utilities volume of
 * POSIX.1 to which the implementation conforms. For implementations
 * conforming to POSIX.1-2008, the value shall be 200809L.
 */
#define _POSIX2_VERSION 200809L

/**
 * Get the process ID.
 * The getpid() function shall return the process ID of the calling process.
 * @return The getpid() function shall always be successful and no return
 *         value is reserved to indicate an error.
 */
extern C pid_t getpid();

/**
 * Read from a file
 * @param fildes The read() function shall attempt to read nbyte bytes from the file
 *               associated with the open file descriptor, fildes, into the buffer
 *               pointed to by buf.
 * @param buf The read() function shall attempt to read nbyte bytes from the file
 *            associated with the open file descriptor, fildes, into the buffer
 *            pointed to by buf.
 * @param nbyte If the value of nbyte is greater than {SSIZE_MAX}, the result is implementation-defined.
 * @return Upon successful completion, these functions shall return a non-negative integer
 *         indicating the number of bytes actually read. Otherwise, the functions shall
 *         return -1 and set errno to indicate the error.
 */
extern C ssize_t read(int fildes, void *buf, size_t nbyte);

/**
 * @brief Write on a file.
 *
 * The write() function shall attempt to write nbyte bytes from
 * the buffer pointed to by buf to the file associated with the
 * open file descriptor, fildes.
 *
 * @param fildes File descriptor.
 * @param buf Input buffer.
 * @param nbyte Maximum number of bytes to write.
 * @return Upon successful completion, these functions shall return the number
 *         of bytes actually written to the file associated with fildes. This
 *         number shall never be greater than nbyte. Otherwise, -1 shall be
 *         returned and errno set to indicate the error.
 */
extern C ssize_t write(int fildes, const void *buf, size_t nbyte);

/**
 * Close a file descriptor
 * @param fildes The close() function shall deallocate the file descriptor indicated by fildes.
 * @return Upon successful completion, 0 shall be returned; otherwise, -1 shall
 *         be returned and errno set to indicate the error.
 *
 */
extern C int close(int fildes);

/**
 * @brief Move the read/write file offset.
 *
 * The lseek() function shall set the file offset for the open file
 * description associated with the file descriptor fildes, as follows:
 *
 * If whence is SEEK_SET, the file offset shall be set to offset bytes.
 * If whence is SEEK_CUR, the file offset shall be set to its current location plus offset.
 * If whence is SEEK_END, the file offset shall be set to the size of the file plus offset.
 *
 * @param fildes File descriptor.
 * @param offset New file offset.
 * @param whence Determines how to modify the file offset pointer.
 * @return Upon successful completion, the resulting offset, as measured in bytes
 *         from the beginning of the file, shall be returned. Otherwise, (off_t)-1
 *         shall be returned, errno shall be set to indicate the error, and the file
 *         offset shall remain unchanged.
 */
extern C off_t lseek(int fildes, off_t offset, int whence);

/**
 * @brief Execute a file.
 *
 * The exec family of functions shall replace the current process image with a
 * new process image. The new image shall be constructed from a regular,
 * executable file called the new process image file. There shall be no return
 * from a successful exec, because the calling process image is overlaid by the
 * new process image.
 *
 * @param path File to execute.
 * @param argv The argument argv is an array of character pointers to
 *             null-terminated strings. The application shall ensure that the
 *             last member of this array is a null pointer. These strings shall
 *             constitute the argument list available to the new process image.
 *             The value in argv[0] should point to a filename that is associated
 *             with the process being started by one of the exec functions.
 * @return If one of the exec functions returns to the calling process image,
 *         an error has occurred; the return value shall be -1, and errno shall
 *         be set to indicate the error.
 *
 */
extern C int execv(const char *path, const char *argv[]);

/**
 * @}
 */

#endif /* __LIBPOSIX_UNISTD_H */
