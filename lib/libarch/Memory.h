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

#ifndef __LIBARCH_MEMORY_H
#define __LIBARCH_MEMORY_H

#include <FreeNOS/System.h>
#include <Types.h>
#include <Macros.h>
#include <BitOperations.h>
#include <BitArray.h>

/**
 * Virtual memory abstract interface.
 */
class Memory
{
  public:

    /**
     * Memory access flags.
     *
     * The actual values of this enum depend on the architecture
     * specific constants of the virtual memory page table implementation.
     */
    typedef enum Access
    {
        None       = PAGE_NONE,
        Present    = PAGE_PRESENT,
        Readable   = PAGE_READ,
        Writable   = PAGE_WRITE,
        Executable = PAGE_EXEC,
        User       = PAGE_USER,
        Pinned     = PAGE_PIN,
        Reserved   = PAGE_RESERVE
    }
    Access;

    /**
     * Memory range.
     */
    typedef struct Range
    {
        Address virt;   /**< Virtual address. */
        Address phys;   /**< Physical address. */
        Size    size;   /**< Size in number of bytes. */
        Access  access; /**< Page access flags. */
    }
    Range;

    /**
     * Constructor.
     *
     * @param pageDirectory Physical address of the page directory.
     * @param memoryMap BitArray of physical memory page allocations.
     */
    Memory(Address pageDirectory, BitArray *memoryMap);

    /**
     * Destructor
     */
    virtual ~Memory();

    /**
     * Map a physical page to a virtual address.
     *
     * @param phys Physical address.
     * @param virt Virtual address or ZERO to use the first unused page.
     * @param access Page entry protection flags.
     * @return Mapped virtual address.
     */     
    virtual Address map(Address phys,
                        Address virt = ZERO,
                        Access access = Present | User | Readable | Writable) = 0;

    /**
     * Unmap a virtual address.
     *
     * This function removes a virtual to physical memory
     * mapping without deallocating any physical memory.
     *
     * @param virt Virtual address to unmap.
     */
    virtual void unmap(Address virt) = 0;

    /**
     * Map a range of physical pages to virtual addresses.
     *
     * @param range Range object describing the range of physical pages.
     * @return First virtual memory address of the mapping.
     */
    virtual Address mapRange(Range *range);

    /**
     * Unmaps a range of virtual memory.
     *
     * @param range Range object describing the range of virtual addresses.
     */
    virtual void unmapRange(Range *range);

    /**
     * Translate virtual address to physical address.
     *
     * @param virt Virtual address to lookup.
     * @return Physical address of the virtual address given.
     */
    virtual Address lookup(Address virt) = 0;

    /**
     * Verify protection access flags.
     *
     * @param virt Virtual address start to validate.
     * @param size Number of bytes to check.
     * @param flags Page protection flags which must be set.
     * @return True if flags are set on the given range, false otherwise.
     */
    virtual bool access(Address virt,
                        Size size,
                        Access flags = Present | User | Readable) = 0;

    /**
     * Release a memory page mapping.
     *
     * @param virt Virtual address of the page to release.
     */
    virtual void release(Address virt) = 0;

    /**
     * Release a range of physical memory by its virtual memory pages.
     *
     * @param range Range object describing the range of physical pages to release.
     */
    virtual void releaseRange(Range *range);

    /** 
     * Deallocate all associated physical memory.
     */
    virtual void releaseAll() = 0;

    /**
     * Find unused virtual page range.
     *
     * This function finds a contigeous block of a given size
     * of virtual memory which is unused and then returns
     * the virtual address of the first page in the block.
     *
     * @param size Number of bytes requested to be free.
     * @return Virtual address of the first page.
     */
    virtual Address findFree(Size size) = 0;

  protected:

    /** Physical memory allocation bitmap */
    BitArray *m_memoryMap;
};

#endif /* __LIBARCH_MEMORY_H */
