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

#ifndef __LIBARCH_ARM64_PAGETABLE_H
#define __LIBARCH_ARM64_PAGETABLE_H

#include <FreeNOS/System.h>
#include <Types.h>
#include <MemoryContext.h>

/** Forward declaration */
class SplitAllocator;
class ARM64Paging;

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
 * ARM64 page table
 */
class ARM64PageTable
{
  public:

    /**
     * Map a virtual address to a physical address.
     *
     * @param virt Virtual address.
     * @param phys Physical address.
     * @param access Memory access flags.
     * @param alloc Physical memory allocator for extra page tables.
     *
     * @return Result code
     */
    MemoryContext::Result map(Address virt,
                              Address phys,
                              Memory::Access access,
                              SplitAllocator *alloc);

    /**
     * Map a contigous range of virtual memory to physical memory.
     *
     * This function can map at the 1 block which depends on the level of table.
     *
     * @param range Virtual to physical memory range.
     * @param alloc Physical memory allocator for extra page tables.
     *
     * @return Result code
     */
    MemoryContext::Result mapBlock(Memory::Range range,
                                   SplitAllocator *alloc);

    /**
     * Map a contigous range of virtual memory to physical memory, using next level table
     *
     * This function can map at the 1 block which depends on the level of table.
     *
     * @param range Virtual to physical memory range.
     * @param alloc Physical memory allocator for extra page tables.
     *
     * @return Result code
     */
    MemoryContext::Result mapBlock2(Memory::Range range,
                                   SplitAllocator *alloc);

    /**
     * Remove virtual address mapping.
     *
     * @param virt Virtual address.
     * @param alloc Physical memory allocator
     *
     * @return Result code
     */
    MemoryContext::Result unmap(Address virt,
                                SplitAllocator *alloc);

    /**
     * Translate virtual address to physical address.
     *
     * @param virt Virtual address to lookup as input
     * @param phys Physical address corresponding to the virtual address
     * @param alloc Physical memory allocator
     *
     * @return Result code
     */
    MemoryContext::Result translate(Address virt,
                                    Address *phys,
                                    SplitAllocator *alloc) const;

    /**
     * Get Access flags for a virtual address.
     *
     * @param virt Virtual address to get Access flags for.
     * @param access MemoryAccess object pointer.
     * @param alloc Physical memory allocator
     *
     * @return Result code.
     */
    MemoryContext::Result access(Address virt,
                                 Memory::Access *access,
                                 SplitAllocator *alloc) const;

    /**
     * Release memory blocks.
     *
     * @param range Memory range of the sections to release
     * @param alloc Memory allocator to release memory from
     * @param tablesOnly Set to true to only release page tables and not mapped pages.
     *
     * @return Result code.
     */
    MemoryContext::Result releaseBlock(const Memory::Range range,
                                         SplitAllocator *alloc,
                                         const bool tablesOnly);

    /**
     * Release range of memory.
     *
     * @param range Memory range input
     * @param alloc Memory allocator to release memory from
     *
     * @return Result code
     */
    MemoryContext::Result releaseRange(const Memory::Range range,
                                       SplitAllocator *alloc);

  private:

    /**
     * Release a single physical page
     *
     * @param alloc Memory allocator to release memory from
     * @param phys Physical address to release
     */
    inline void releasePhysical(SplitAllocator *alloc,
                                const Address phys);

    /**
     * Retrieve next level page table
     *
     * @param virt Virtual address to fetch page table for
     * @param alloc Physical memory allocator
     *
     * @return Next level page table
     */
    ARM64PageTable * getNextTable(Address virt,
                                    SplitAllocator *alloc) const;

    void setNextTable(Address virt, Address tbl, SplitAllocator *alloc);

    /**
     * Convert Memory::Access to first level page table flags.
     *
     * @param access Memory access flags to convert
     *
     * @return Page table access flags
     */
    u32 flags(Memory::Access access) const;

  private:

    friend class ARM64Paging;

    /** Array of page table entries. only support 4KB granule */
    u64 m_tables[512];

    /* Level of page table */
    u8 m_level;
    u8 padding[7];
};

/**
 * @}
 * @}
 * @}
 */

#endif /* __LIBARCH_ARM64_PAGETABLE_H */
