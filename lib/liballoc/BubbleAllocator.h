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
	 * Empty class constructor.
	 */
	BubbleAllocator();

	/**
	 * Class constructor.
	 * @param start Memory address to start allocating from.
	 * @param size Maximum size of the memory region to allocate from.
	 */
	BubbleAllocator(Address start, Size size);

        /**
         * Allocate a block of memory.
	 * @param size Amount of memory in bytes to allocate on input. 
	 *             On output, the amount of memory in bytes actually allocated.
         * @return New memory block on success and ZERO on failure.
         */
        Address allocate(Size *size);

        /**
         * Does nothing as we are a bubble.
         * @param address Points to memory previously returned by allocate().
         * @see allocate
         */
        void release(Address addr);
	
        /** 
         * Use the given memory address and size for the allocator. 
         * Allocators are free to use multiple memory regions for allocation. 
         * @param address Memory address to use. 
         * @param size Size of the memory address. 
	 */
	void region(Address addr, Size size);

    private:
    
	/** Memory region to allocate from. */
	u8 *start;
	
	/** Current "top" of the growing bubble. */
	u8 *current;
	
	/** Size of the memory region. */
	Size size;
};

/** 
 * @}
 */

#endif /* __LIBALLOC_BUBBLEALLOCATOR_H */
