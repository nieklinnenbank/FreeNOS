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

#ifndef __ERROR_H
#define __ERROR_H

/** Reports a success operation. */
#define ESUCCESS	0

/** Target could not be found. */
#define ENOSUCH		-1

/** Invalid argument given. */
#define EINVALID	-2

/** Requested operation is not supported. */
#define ENOSUPPORT	-3

/** Memory address protection fault. */
#define EFAULT		-4

/** Permission denied. */
#define EACCESS		-5

/** Not enough memory. */
#define ENOMEM		-6

#endif /* __ERROR_H */
