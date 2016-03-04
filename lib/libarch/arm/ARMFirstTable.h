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

#ifndef __LIBARCH_ARM_FIRSTTABLE_H
#define __LIBARCH_ARM_FIRSTTABLE_H

#include <FreeNOS/System.h>
#include <Types.h>
#include <MemoryContext.h>
#include "ARMSecondTable.h"

/** Forward declaration */
class SplitAllocator;

/**
 * ARM first level page table
 */
class ARMFirstTable
{
  public:

    /**
     * Map a virtual address to a physical address.
     *
     * @param virt Virtual address.
     * @param phys Physical address.
     * @param access Memory access flags.
     * @param alloc Physical memory allocator for extra page tables.
     * @return Result code
     */
    MemoryContext::Result map(Address virt,
                              Address phys,
                              Memory::Access access,
                              SplitAllocator *alloc);

    /**
     * Map a contigous range of virtual memory to physical memory.
     *
     * This function can map at the granularity of 1 megabyte memory chunks.
     *
     * @param range Virtual to physical memory range.
     * @param alloc Physical memory allocator for extra page tables.
     * @return Result code
     */
    MemoryContext::Result mapLarge(Memory::Range range,
                                   SplitAllocator *alloc);

    /**
     * Remove virtual address mapping.
     *
     * @param virt Virtual address.
     * @return Result code
     */
    MemoryContext::Result unmap(Address virt,
                                SplitAllocator *alloc);

    /**
     * Translate virtual address to physical address.
     *
     * @param virt Virtual address to lookup on input, physical address on output.
     * @return Result code
     */
    MemoryContext::Result translate(Address virt,
                                    Address *phys,
                                    SplitAllocator *alloc);

    /**
     * Get Access flags for a virtual address.
     *
     * @param virt Virtual address to get Access flags for.
     * @param access MemoryAccess object pointer.
     * @return Result code.
     */
    MemoryContext::Result access(Address virt,
                                 Memory::Access *access,
                                 SplitAllocator *alloc);

  private:

    ARMSecondTable * getSecondTable(Address virt, SplitAllocator *alloc);

    /**
     * Convert Memory::Access to first level page table flags.
     */
    u32 flags(Memory::Access access);

    /** Array of page table entries. */
    u32 m_tables[4096];
};

#endif /* __LIBARCH_ARM_FIRSTTABLE_H */
