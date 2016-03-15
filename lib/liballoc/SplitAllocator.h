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
#include "Allocator.h"
#include "BitAllocator.h"

/** 
 * @defgroup liballoc liballoc 
 * @{ 
 */

/**
 * Allocator which separates kernel mapped low-memory and higher memory.
 */
class SplitAllocator : public Allocator
{
  public:

    /**
     * Class constructor.
     *
     * @param low Lower physical memory.
     * @param high Higher physical memory.
     */
    SplitAllocator(Memory::Range low, Memory::Range high);

    /**
     * Class destructor.
     */
    virtual ~SplitAllocator();

    /**
     * Get memory size.
     *
     * @return Size of memory owned by the Allocator.
     */
    virtual Size size();

    /**
     * Get memory available.
     *
     * @return Size of memory available by the Allocator.
     */
    virtual Size available();

    /**
     * Allocate memory.
     *
     * @param size Amount of memory in bytes to allocate on input. 
     *             On output, the amount of memory in bytes actually allocated.
     * @param addr Output parameter which contains the address
     *             allocated on success.
     * @param align Alignment of the required memory or use ZERO for default.
     * @return Result code
     */
    virtual Result allocate(Size *size, Address *addr, Size align = ZERO);

    /**
     * Allocate address.
     *
     * @param addr Allocate a specific address.
     * @return Result value.
     */
    Result allocate(Address addr);

    /**
     * Allocate from lower memory.
     *
     * @param size Amount of memory in bytes to allocate.
     * @param addr Output parameter which contains the address
     *             allocated on success.
     * @param align Alignment of the required memory or use ZERO for default.
     * @return Result code
     */
    virtual Result allocateLow(Size size, Address *addr, Size align = ZERO);

    /**
     * Allocate from high memory.
     *
     * @param size Amount of memory in bytes to allocate.
     * @param addr Output parameter which contains the address
     *             allocated on success.
     * @param align Alignment of the required memory or use ZERO for default.
     * @return Result code
     */
    virtual Result allocateHigh(Size size, Address *addr, Size align = ZERO);

    /**
     * Release memory.
     *
     * @param addr Points to memory previously returned by allocate().
     * @return Result value.
     *
     * @see allocate
     */
    virtual Result release(Address addr);

    /**
     * Convert the given physical address to lower virtual accessible address.
     */
    void * toVirtual(Address phys);

  private:

    /** Physical memory allocator. */
    BitAllocator *m_alloc;

    /** Low memory */
    Memory::Range m_low;

    /** High memory */
    Memory::Range m_high;
};

/**
 * @}
 */

#endif /* __LIBALLOC_SPLITALLOCATOR_H */
