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

#ifndef __ARM_MEMORY_H
#define __ARM_MEMORY_H

/**
 * @defgroup armkernel kernel (arm)
 * @{
 */

/** ARM uses 4K pages. */
#define PAGESIZE        4096

/** Memory address alignment. */
#define MEMALIGN        4

/** Marks a page entry present. */
#define PAGE_PRESENT    1

/** Marks a page entry read/write. */
#define PAGE_RW         1

/** Marks a page accessible by user programs. */
#define PAGE_USER       1

/** Pinned pages cannot be released. */
#define PAGE_PINNED     1

/** This page has been marked for temporary operations. */
#define PAGE_MARKED     1

/** Page has been reserved for future use. */
#define PAGE_RESERVED   1

/**
 * @}
 */

#endif /* __ARM_MEMORY_H */
