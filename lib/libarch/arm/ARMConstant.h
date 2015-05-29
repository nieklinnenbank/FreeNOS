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

#ifndef __LIBARCH_ARM_CONSTANT_H
#define __LIBARCH_ARM_CONSTANT_H

/**
 * ARM CPU Constants
 * @{
 */

/** ARM is little endian. */
#define CPU_LITTLE_ENDIAN 1

/**
 * @}
 */

/**
 * ARM Memory Constants.
 * @{
 */

/** Page entry bit shift. */
#define PAGESHIFT       12

/** Page table bit shift. */
#define DIRSHIFT        22

/** Intel uses 4K pages. */
#define PAGESIZE        4096

/** Number of entries in the page directory. */
#define PAGEDIR_MAX     1024

/** Number of entries in a page table. */
#define PAGETAB_MAX     1024

/** Mask to find the page. */
#define PAGEMASK        0xfffff000 

/** Memory address alignment. */
#define MEMALIGN        4

/** Local page directory in virtual memory */
#define PAGEDIR_LOCAL   (1024 * 1024 * 4)

/** Kernel stack address. */
#define KERNEL_STACK    0xd0000000

/** User stack address. */
#define USER_STACK      0xc0000000

/** User heap address. */
#define USER_HEAP       0xb0000000

#define PAGE_NONE       0

/** Marks a page entry present. */
#define PAGE_PRESENT    1

#define PAGE_READ       0

#define PAGE_EXEC       0

/** Marks a page entry read/write. */
#define PAGE_WRITE      2

/** Marks a page accessible by user programs (ring 3). */
#define PAGE_USER       4

/** Pinned pages cannot be released by the physical memory allocator. */
#define PAGE_PIN        (1 << 9)

/** Page has been reserved in the virtual address space. */
#define PAGE_RESERVE    (1 << 11)

/**
 * @}
 */

#endif /* __LIBARCH_ARM_CONSTANT_H */
