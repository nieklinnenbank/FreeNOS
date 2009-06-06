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

#ifndef __LIBPOSIX_FCNTL_H
#define __LIBPOSIX_FCNTL_H

#include <Macros.h>
#include "sys/types.h"

/**                                                                                                                                                                                                     
 * @defgroup libposix libposix (POSIX.1-2008)
 * @{
 */

/**
 * Open file relative to directory file descriptor.
 * @param path The path argument points to a pathname naming the file.
 * @param oflag Values for oflag are constructed by a bitwise-inclusive OR of
 *              flags from the following list, defined in <fcntl.h>.
 * @return Upon successful completion, these functions shall open the file and
 *         return a non-negative integer representing the lowest numbered unused
 *         file descriptor. Otherwise, these functions shall return -1 and set
 *         errno to indicate the error. If - is returned, no files shall be created or modified.
 */
extern C int open(const char *path, int oflag, ...);

/**
 * @}
 */

#endif /* __LIBPOSIX_UNISTD_H */
