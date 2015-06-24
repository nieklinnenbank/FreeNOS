/*
 * Copyright (C) 2009 Niek Linnenbank
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

#ifndef __LIBALLOC_BUBBLEALLOCATOR_H
#define __LIBALLOC_BUBBLEALLOCATOR_H

#include <Types.h>
#include "Allocator.h"

/** 
 * @defgroup liballoc liballoc 
 * @{ 
 */

/**
 * Keeps growing allocated memory, and can't actually free memory (hence the name).
 */
class BubbleAllocator : public Allocator
{
  public:

    /**
     * Class constructor.
     *
     * @param start Memory address to start allocating from.
     * @param size Maximum size of the memory region to allocate from.
     */
    BubbleAllocator(Address start, Size size);

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
     * Allocate memory
     *
     * @param size Amount of memory in bytes to allocate on input. 
     *             On output, the amount of memory in bytes actually allocated.
     * @param addr Output parameter which contains the address
     *             allocated on success.
     * @param align Alignment of the required memory or use ZERO for default.
     * @return New memory block on success and ZERO on failure.
     */
    virtual Result allocate(Size *size, Address *addr, Size align = ZERO);

    /**
     * Release memory.
     *
     * Does nothing for BubbleAllocator.
     *
     * @param address Points to memory previously returned by allocate().
     *
     * @see allocate
     */
    virtual Result release(Address addr);
    
  private:
    
    /** Memory region to allocate from. */
    u8 *m_start;
    
    /** Current "top" of the growing bubble. */
    u8 *m_current;
    
    /** Size of the memory region. */
    Size m_size;
};

/** 
 * @}
 */

#endif /* __LIBALLOC_BUBBLEALLOCATOR_H */
