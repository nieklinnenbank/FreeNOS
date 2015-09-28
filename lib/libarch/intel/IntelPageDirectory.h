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

#ifndef __LIBARCH_INTEL_PAGEDIRECTORY_H
#define __LIBARCH_INTEL_PAGEDIRECTORY_H

#include <Types.h>
#include <MemoryContext.h>
#include "IntelPageTable.h"

/** Forward declaration */
class SplitAllocator;

/**
 * Intel page directory implementation.
 */
class IntelPageDirectory
{
  public:

    /**
     * Copy mappings from another directory.
     */
    MemoryContext::Result copy(IntelPageDirectory *directory,
                               Address from,
                               Address to);

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

    IntelPageTable * getPageTable(Address virt, SplitAllocator *alloc);

    /**
     * Convert Memory::Access to page directory flags.
     */
    u32 flags(Memory::Access access);

    /** Array of page directory entries. */
    u32 m_tables[1024];
};

#endif /* __LIBARCH_INTEL_PAGEDIRECTORY_H */
