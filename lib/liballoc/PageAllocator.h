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

#ifndef __LIBALLOC_PAGEALLOCATOR_H
#define __LIBALLOC_PAGEALLOCATOR_H

#include <arch/Memory.h>
#include <Types.h>
#include "Allocator.h"

/** 
 * @defgroup liballoc liballoc 
 * @{ 
 */

/** Minimum size required to allocate. */
#define PAGEALLOC_MINIMUM (PAGESIZE * 2)

/**
 * Allocates virtual memory using the memory server.
 */
class PageAllocator : public Allocator
{
    public:

	/**
	 * Empty class constructor.
	 */
	PageAllocator();

	/**
	 * Class constructor.
	 * @param size Initial size in bytes.
	 */
	PageAllocator(Size size);

	/**
	 * Copy constructor.
	 * @param p PageAllocator instance pointer.
	 */
	PageAllocator(PageAllocator *p);

        /**
         * Allocates a new memory page, if neccessary.
	 * @param size Amount of memory in bytes to allocate on input. 
	 *             On output, the amount of memory in bytes actually allocated.
         * @return New memory block on success and ZERO on failure.
         */
        Address allocate(Size *size);

        /**
         * Unmaps memory pages, if possible.
         * @param address Points to memory previously returned by allocate().
         * @see allocate
         */
        void release(Address addr);

	/**
	 * Get start address of the heap.
	 * @return Start heap address.
	 */
	Address getHeapStart()
	{
	    return heapStart;
	}

    private:

	/** Start of the current heap. */
	Address heapStart;
	
	/** End of the heap. */
	Address heapEnd;
	
	/** Total number of bytes allocated. */
	Size allocated;
};

/**
 * @}
 */

#endif /* __LIBALLOC_PAGEALLOCATOR_H */
