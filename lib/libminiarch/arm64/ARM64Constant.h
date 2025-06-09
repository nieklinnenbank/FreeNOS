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

#ifndef __LIBARCH_ARM64_CONSTANT_H
#define __LIBARCH_ARM64_CONSTANT_H

/* FIXME: deprecated*/
#define DIRSHIFT        20
/** Mask for large 1MiB section mappings. */
#define SECTIONMASK     0xfff00000

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libarch
 * @{
 *
 * @addtogroup libarch_arm
 * @{
 */

/**
 * ARM CPU Constants
 * @{
 */

/**
 * @}
 */

/**
 * ARM Program Status Register (CPSR)
 * @{
 */

#define USR_MODE    16
#define IRQ_MODE    18
#define FIQ_MODE    17
#define SVC_MODE    19
#define ABT_MODE    23
#define UND_MODE    27
#define SYS_MODE    31
#define HYP_MODE    0x1a
#define MODE_MASK   0x1f
#define IRQ_BIT         (1 << 7)
#define FIQ_BIT         (1 << 6)
#define ASYNC_ABORT_BIT (1 << 8)

/**
 * @}
 */

/**
 * ARM Model-Specific Instructions
 * @{
 */

/** msr ELR_hyp, lr */
#define ELR  .long 0xe12ef30e

/** eret */
#define ERET .long 0xe160006e

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
 * In ARM64, the shifts of each level vary based on granule. Assume 4K granule.
 *
 * @see https://developer.arm.com/documentation/101811/0104/Translation-granule/The-starting-level-of-address-translation
 */
#define L1_DIRSHIFT        30UL
#define L2_DIRSHIFT        21UL
#define L3_DIRSHIFT        12UL
#define DIR_MASK           0x1FFUL

/** ARM uses 4K pages. */
#define PAGESIZE        4096

/**
 * Number of entries in the each level page table.
 *
 * TTBR0 is the first-level page table for the application.
 * TTBR1 is always 16KB, the first-level page table of the kernel/OS.
 */
#define PAGEDIR_MAX     512
#define PAGEDIR_SIZE    (PAGEDIR_MAX * sizeof(u64))

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
#define PAGETAB_SPAN    (PAGETAB_MAX*PAGESIZE)

/** Mask to find the page. */
#define PAGEMASK        0x3FFFFFFFFF000UL 

/** Mask for large 1GB block mappings. */
#define L1_BLOCK_MASK     0x3FFFFC0000000UL
#define L1_BLOCK_SIZE     0x40000000UL
#define L1_BLOCK_RANGE    (L1_BLOCK_SIZE-1)
/** Mask for large 2MB block mappings. */
#define L2_BLOCK_MASK     0x3FFFFFFE00000UL
#define L2_BLOCK_SIZE     0x200000UL
#define L2_BLOCK_RANGE    (L2_BLOCK_SIZE-1)

/** Memory address alignment. */
#define MEMALIGN        4

#define contain_flags(val, flags) (((val)&flags)==flags)

/**
 * @}
 * @}
 * @}
 * @}
 */

#endif /* __LIBARCH_ARM64_CONSTANT_H */
