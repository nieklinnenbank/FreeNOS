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

#include <Macros.h>
#include "errno.h"

Size errno USED = 0;

char * error_map[] USED =
{
    [-ESUCCESS]   = "Success",
    [-ENOSUCH]    = "No such file or directory",
    [-EINVALID]   = "Invalid argument",
    [-ENOSUPPORT] = "Operation is not supported",
    [-EFAULT]     = "Memory fault",
    [-EACCESS]    = "Permission denied",
    [-ENOMEM]     = "Not enough memory",
    [-ERANGE]     = "Number out of range",
};
