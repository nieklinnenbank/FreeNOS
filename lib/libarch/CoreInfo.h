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

#ifndef __LIBARCH_COREINFO_H
#define __LIBARCH_COREINFO_H

#include <Macros.h>
#include <Types.h>
#include <Memory.h>

#define KERNEL_PATHLEN 64

typedef struct CoreInfo
{
    uint booted;
    uint coreId;
    Memory::Range memory;
    char kernel[KERNEL_PATHLEN];

} CoreInfo;

#endif /* __LIBARCH_COREINFO_H */
