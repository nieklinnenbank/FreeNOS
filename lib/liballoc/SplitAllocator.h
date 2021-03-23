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

#ifndef __LIBALLOC_SPLITALLOCATOR_H
#define __LIBALLOC_SPLITALLOCATOR_H

#include <Types.h>
#include <Callback.h>
#include "Allocator.h"
#include "BitAllocator.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup liballoc
 * @{
 */

/**
 * Allocator which separates kernel mapped memory at virtual and physical addresses.
 */
class SplitAllocator : public Allocator
{
  public:

    /**
     * Class constructor.
     *
     * @param physRange Block of physical continguous memory to be managed.
     * @param virtRange Same block of memory in the virtual address space.
     * @param pageSize Size of a single memory page.
     */
    SplitAllocator(const Range physRange,
                   const Range virtRange,
                   const Size pageSize);

    /**
     * Get memory available.
     *
     * @return Size of memory available by the Allocator.
     */
    virtual Size available() const;

    /**
     * Allocate physical memory.
     *
     * @param args Contains the requested size and alignment on input.
     *             The alignment value must be a multiple of the pageSize.
     *             On output, contains the actual allocated address.
     *
     * @return Result value.
     */
    virtual Result allocate(Range & args);

    /**
     * Allocate sparse (non-contiguous) physical memory.
     *
     * @param range Contains the requested size on input.
     *              The alignment and address values are unused.
     * @param function Called for each successful allocation of a page.
     *
     * @return Result value.
     */
    Result allocateSparse(const Range & range, CallbackFunction *function);

    /**
     * Allocate physical/virtual memory.
     *
     * @param phys Contains the requested size and alignment on input.
     *             The alignment value must be a multiple of the pageSize.
     *             On output, contains the actual allocated physical address.
     * @param virt Contains the allocated memory translated for virtual addressing.
     *
     * @return Result code
     */
    Result allocate(Range & phys, Range & virt);

    /**
     * Allocate one physical memory page.
     *
     * @param addr Physical memory page address
     *
     * @return Result code
     */
    Result allocate(const Address addr);

    /**
     * Release memory page.
     *
     * @param addr Physical memory address of page to release.
     *
     * @return Result value.
     *
     * @see allocate
     */
    virtual Result release(const Address addr);

    /**
     * Convert Address to virtual pointer.
     *
     * @param phys Physical address
     *
     * @return Virtual address
     */
    Address toVirtual(const Address phys) const;

    /**
     * Convert Address to physical pointer.
     *
     * @param virt Virtual address
     *
     * @return Physical address
     */
    Address toPhysical(const Address virt) const;

    /**
     * Check if a physical page is allocated.
     *
     * @return True if allocated, false otherwise.
     */
    bool isAllocated(const Address page) const;

  private:

    /** Physical memory allocator. */
    BitAllocator m_alloc;

    /** Virtual memory range to manage. */
    const Range m_virtRange;

    /** Size of a memory page. */
    const Size m_pageSize;
};

/**
 * @}
 * @}
 */

#endif /* __LIBALLOC_SPLITALLOCATOR_H */
