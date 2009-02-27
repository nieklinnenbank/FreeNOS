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

#ifndef __LIBPOSIX_TYPES_H
#define __LIBPOSIX_TYPES_H

#include <arch/Process.h>
#include <Types.h>

/**
 * @defgroup posix POSIX.1-2008
 * @{
 */

/** Used for process IDs and process group IDs. */
typedef ProcessID pid_t;

/** Used for sizes of objects. */
typedef Size size_t;

/** Used for a count of bytes or an error indication. */
typedef slong ssize_t;

/**
 * @}
 */

#endif /* __LIBPOSIX_TYPES_H */
