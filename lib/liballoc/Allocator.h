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

#ifndef __LIBALLOC_ALLOCATOR_H
#define __LIBALLOC_ALLOCATOR_H
#ifndef __ASSEMBLER__

#include <Macros.h>
#include <Types.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnew-returns-null"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup liballoc
 * @{
 */

/**
 * Memory Allocator.
 *
 * This class defines an abstract memory allocator. Each
 * class which derives must provide functions for memory
 * allocation and release. The memory allocators form a hierarcy
 * of parent-child. A parent allocator can provide memory to a child
 * allocator. If a child allocator runs out of memory, it can ask the
 * parent for more memory.
 */
class Allocator
{
  public:

    /**
     * Allocation results.
     */
    enum Result
    {
        Success = 0,
        InvalidAddress,
        InvalidSize,
        InvalidAlignment,
        OutOfMemory
    };

    /**
     * Describes a range of memory.
     */
    typedef struct Range
    {
        Address address; /**< Starting address of the memory range. */
        Size size;       /**< Amount of memory in bytes. */
        Size alignment;  /**< Alignment in bytes or ZERO for default alignment. */
    } Range;

  public:

    /**
     * Default class constructor.
     */
    Allocator();

    /**
     * Class constructor with Range input.
     *
     * @param range Block of continguous memory to manage.
     */
    Allocator(const Range range);

    /**
     * Class destructor.
     */
    virtual ~Allocator();

    /**
     * Makes the given Allocator the default.
     *
     * @param alloc Instance of an Allocator.
     */
    static void setDefault(Allocator *alloc);

    /**
     * Retrieve the currently default Allocator.
     *
     * @return Allocator pointer.
     */
    static Allocator *getDefault();

    /**
     * Set parent allocator.
     *
     * @param parent New parent allocator.
     */
    void setParent(Allocator *parent);

    /**
     * Get parent Allocator
     *
     * @return Pointer to parent Allocator instance or NULL if none.
     */
    Allocator * parent();

    /**
     * Get memory base address for allocations.
     *
     * @return Memory address used as base address
     */
    Address base() const;

    /**
     * Get memory alignment in bytes for allocations.
     *
     * @return Memory alignment value in bytes
     */
    Size alignment() const;

    /**
     * Get memory size.
     *
     * @return Size of memory owned by the Allocator.
     */
    virtual Size size() const;

    /**
     * Get memory available.
     *
     * @return Size of memory available by the Allocator.
     */
    virtual Size available() const;

    /**
     * Allocate memory.
     *
     * @param range Contains the requested size and alignment on input.
     *              On output, contains the actual allocated address.
     *
     * @return Result value.
     */
    virtual Result allocate(Range & range);

    /**
     * Release memory.
     *
     * @param addr Points to memory previously returned by allocate().
     *
     * @return Result value.
     *
     * @see allocate
     */
    virtual Result release(const Address addr);

  protected:

    /**
     * Align memory address.
     *
     * Any alignment corrections on the input address will result
     * in an address which is higher than the input address.
     *
     * @param addr Input address which need to be aligned.
     * @param boundary Boundary size to align the address for.
     *
     * @return Aligned Address.
     */
    Address aligned(const Address addr, const Size boundary) const;

  private:

    /** Points to the default Allocator for new()/delete(). */
    static Allocator *m_default;

    /** Our parent Allocator, if any. */
    Allocator *m_parent;

    /** Range of memory that this Allocator manages */
    Range m_range;
};

/**
 * @name Dynamic memory allocation.
 * @{
 */

#ifndef __HOST__

/**
 * Allocate new memory.
 *
 * @param sz Amount of memory to allocate.
 */
inline void * operator new(__SIZE_TYPE__ sz)
{
    Allocator::Range alloc_args;

    alloc_args.size = sz;
    alloc_args.alignment = 0;

    if (Allocator::getDefault()->allocate(alloc_args) == Allocator::Success)
        return (void *) alloc_args.address;
    else
        return (void *) NULL;
}

/**
 * Allocate memory for an array.
 *
 * @param sz Amount of memory to allocate.
 */
inline void * operator new[](__SIZE_TYPE__ sz)
{
    Allocator::Range alloc_args;

    alloc_args.size = sz;
    alloc_args.alignment = 0;

    if (Allocator::getDefault()->allocate(alloc_args) == Allocator::Success)
        return (void *) alloc_args.address;
    else
        return (void *) NULL;
}

/**
 * Free memory back to the current Allocator.
 *
 * @param mem Points to memory to release.
 */
inline void operator delete (void *mem)
{
    Allocator::getDefault()->release((Address)mem);
}

/**
 * Uses the Heap class to free memory, with the delete[] operator.
 *
 * @param mem Points to memory to release.
 */
inline void operator delete[] (void *mem)
{
    Allocator::getDefault()->release((Address)mem);
}

#endif /* __HOST__ */

/**
 * Let the new() operator return the given memory address.
 *
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

/**
 * @}
 * @}
 */

#endif /* __ASSEMBLER__ */
#endif /* __LIBALLOC_ALLOCATOR_H */
