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

#ifndef __IMG_BOOTENTRY_H
#define __IMG_BOOTENTRY_H

#include <ExecutableFormat.h>

/** Maximum memory regions. */
#define MAX_REGIONS	16

/**
 * Executable for use inside a BootImage.
 */
typedef struct BootEntry
{
    /** Pointer to the executable format. */
    ExecutableFormat *format;
    
    /** Memory regions extracted from the program. */
    MemoryRegion regions[MAX_REGIONS];
    
    /** Number of memory regions. */
    Size numRegions;
}
BootEntry;

#endif /* __IMG_BOOTENTRY_H */
