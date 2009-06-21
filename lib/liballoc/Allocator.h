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

#ifndef __LIBALLOC_ALLOCATOR_H
#define __LIBALLOC_ALLOCATOR_H
#ifndef __ASSEMBLER__

#include <Types.h>

/**
 * @defgroup liballoc liballoc
 * @{
 */

/**
 * Memory allocator class.
 */
class Allocator
{
    public:

	/**
	 * Class constructor.
	 */
	Allocator();

	/**
	 * Class destructor.
	 */
	virtual ~Allocator();

        /**
         * Allocate a block of memory.
         * @param size Amount of memory in bytes to allocate on input.
	 *             On output, the amount of memory in bytes actually allocated.
         * @return Pointer to the new memory block on success
         *         and ZERO on failure.
         */
        virtual Address allocate(Size *size) = 0;

        /**
         * Free a block of memory.
         * @param addr Points to memory previously returned by allocate().
         * @see allocate
         */
        virtual void release(Address addr) = 0;

	/**
	 * Use the given memory address and size for the allocator.
	 * Allocators are free to use multiple memory regions for allocation.
	 * @param address Memory address to use.
	 * @param size Size of the memory address.
	 */
	virtual void region(Address addr, Size size)
	{
	}

	/**
	 * Sets a new parent Allocator.
	 * @param p New parent allocator.
	 */
	void setParent(Allocator *p)
	{
	    parent = p;
	}

	/**
	 * Makes the given Allocator the default.
	 * @param alloc Instance of an Allocator.
	 */	
	static void setDefault(Allocator *alloc)
	{
	    _default = alloc;
	}

	/**
	 * Retrieve the currently default Allocator.
	 * @return Allocator pointer.
	 */
	static Allocator *getDefault()
	{
	    return _default;
	}

    protected:

	/** Our parent Allocator, if any. */
	Allocator *parent;
	
    private:

	/** Points to the default Allocator. */
	static Allocator *_default;	
};

#ifndef __HOST__

/**
 * @name Dynamic memory allocation.
 * @{
 */

/**
 * Allocate new memory.
 * @param sz Amount of memory to allocate.
 */
inline void * operator new(__SIZE_TYPE__ sz)
{
    return (void *) Allocator::getDefault()->allocate((Size *) &sz);
}

/**
 * Allocate memory for an array.
 * @param sz Amount of memory to allocate.
 */
inline void * operator new[](__SIZE_TYPE__ sz)
{
    return (void *) Allocator::getDefault()->allocate((Size *) &sz);
}

/**
 * Free memory back to the current Allocator.
 * @param mem Points to memory to release.
 */
inline void operator delete (void *mem)
{
    Allocator::getDefault()->release((Address)mem);
}

/**
 * Uses the Heap class to free memory, with the delete[] operator.
 * @param mem Points to memory to release.
 */
inline void operator delete[] (void *mem)
{
    Allocator::getDefault()->release((Address)mem);
}

/**
 * @}
 */

/**
 * @name Absolute memory allocation.
 * @{
 */

/**
 * Let the new() operator return the given memory address.
 * @param sz Size to allocate (ignored).
 * @param addr Memory address to return.
 */
inline void * operator new(__SIZE_TYPE__ sz, Address addr)
{
    return (void *) addr;
}

/**
 * @}
 */

#endif /* __HOST__ */

/**
 * @}
 */

#endif /* __ASSEMBLER__ */
#endif /* __LIBALLOC_ALLOCATOR_H */
