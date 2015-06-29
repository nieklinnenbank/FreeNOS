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

/**
 * Page table bit shift.
 *
 * In ARM, the first-level page directory entry is selected
 * by bits [31:20] from the virtual address, plus two 0 bytes.
 *
 * @see ARM Architecture Reference Manual, page 724.
 */
#define DIRSHIFT        20

/** ARM uses 4K pages. */
#define PAGESIZE        4096

/** Number of entries in the first-level page table.
 *
 * Variable??? Depends on TTBCR, which sets the size of TTBR0.
 * TTBR0 is the first-level page table for the application.
 * TTBR1 is always 16KB, the first-level page table of the kernel/OS.
 */
#define PAGEDIR_MAX     4096
#define PAGEDIR_SIZE    (PAGEDIR_MAX * sizeof(u32))

/**
 * Number of entries in a second-level page table.
 *
 * Second-level page tables are 1KB in size and map 1MB of virtual memory.
 * Second-level page tables must be aligned on 1KB boundaries.
 *
 * @see ARM Architecture Reference Manual, page 734.
 */
#define PAGETAB_MAX     256
#define PAGETAB_SIZE    (PAGETAB_MAX * sizeof(u32))

/** Mask to find the page. */
#define PAGEMASK        0xfffff000 

/** Memory address alignment. */
#define MEMALIGN        4

/** Local page directory in virtual memory */
#define PAGEDIR_LOCAL   (0x04400000)

/**
 * @}
 */

#endif /* __LIBARCH_ARM_CONSTANT_H */
