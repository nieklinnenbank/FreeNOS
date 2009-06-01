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

#ifndef __LIBALLOC_LISTALLOCATOR_H
#define __LIBALLOC_LISTALLOCATOR_H

#include <Types.h>
#include "Allocator.h"

/** 
 * @defgroup liballoc liballoc 
 * @{ 
 */

/** Magic cookie is used to detect heap corruption. */
#define LISTALLOC_MAGIC 0x1234abcd

/** @see MemRegion. */
class MemRegion;

/**
 * Memory block on a Heap.
 */
typedef struct MemBlock
{
    /** Magic cookie. */
    ulong magic;
    
    /** Memory region for this block. */
    MemRegion *region;

    /** Points to the previous and next entry. */
    MemBlock *prev, *next;

    /** Size of the data block (excluding our size). */
    Size size;
    
    /** Marks the block free. */
    u8 free:1;
}
MemBlock;

/**
 * Contigeous memory region block.
 */
typedef struct MemRegion
{
    /** Magic cookie. */
    ulong magic;

    /** Total size in this region. */
    Size size;

    /** Amount of free memory. */
    Size free;

    /** Head of the memory blocks. */
    MemBlock *blocks;

    /** The next region, if any. */
    MemRegion *next;
}
MemRegion;

/**
 * Dynamic memory allocation class.
 */
class ListAllocator : public Allocator
{
    public:

	/**
	 * Empty constructor.
	 */
	ListAllocator();

        /**
         * Class constructor.
         * @param address Where to begin allocating memory.
         * @param size Maximum total amount of memory to allocate.
         */
        ListAllocator(Address addr, Size size);

        /** 
	 * Allocate a block of memory. 
	 * @param size Amount of memory in bytes to allocate on input. 
	 *             On output, the amount of memory in bytes actually allocated. 
	 * @return New memory block on success and ZERO on failure. 
	 */
	Address allocate(Size *size);

	/**	
         * Free a block of memory. 
	 * @param addr Points to memory previously returned by allocate(). 
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

	/**
	 * Find a free MemBlock.
	 * @param size Minimum size of the block.
	 * @param askParent Whether to ask our parent for more if not enough memory.
	 * @return A MemBlock pointer.
	 */
	MemBlock * findFreeBlock(Size size, bool askParent);
    
        /** Memory region list. */
	MemRegion *regions;
};

/**
 * @}
 */

#endif /* __LIBALLOC_LISTALLOCATOR_H */
