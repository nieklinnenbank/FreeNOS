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

#ifndef __LIBALLOC_MEMORY_ALLOCATOR_H
#define __LIBALLOC_MEMORY_ALLOCATOR_H

#include <Types.h>
#include "Allocator.h"

/** 
 * @defgroup liballoc liballoc 
 * @{ 
 */

/**
 * Allocates virtual memory pages directly using Memory.
 *
 * @see Memory
 */
class MemoryAllocator : public Allocator
{
  public:

    /**
     * Class constructor.
     * @param size Initial size in bytes.
     */
    MemoryAllocator(Address base, Size size);

    /**
     * Copy constructor.
     * @param p MemoryAllocator instance pointer.
     */
    MemoryAllocator(MemoryAllocator *p);

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
     * Get base address.
     *
     * @return Base address
     */
    Address base();

    /**
     * Allocate memory.
     *
     * @param size Amount of memory in bytes to allocate on input.
     *             On output, the amount of memory in bytes actually allocated.
     * @param addr Output parameter which contains the address
     *             allocated on success.
     * @param align Alignment of the required memory or use ZERO for default.
     * @return Result value.
     */
    virtual Result allocate(Size *size, Address *addr, Size align = ZERO);

    /**
     * Release memory.
     *
     * @param addr Points to memory previously returned by allocate().
     * @return Result value.
     *
     * @see allocate
     */
    virtual Result release(Address addr);

  private:

    /** Start of the current heap. */
    Address m_base;
    
    /** Total number of bytes allocated. */
    Size m_allocated;

    /** Size of the heap space */
    Size m_size;
};

/**
 * @}
 */

#endif /* __LIBALLOC_MEMORY_ALLOCATOR_H */
