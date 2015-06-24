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

#include <Types.h>
#include <Macros.h>
#include <BitOperations.h>
#include <BitArray.h>

/** Forward declaration */
class BitAllocator;

/**
 * Virtual memory abstract interface.
 */
// TODO: #warning Rename to MemoryContext. and splitup a Memory/MemoryContext.
class Memory
{
  public:

    /**
     * Result codes.
     */
    typedef enum Result
    {
        Success,
        InvalidAddress,
        OutOfMemory
    }
    Result;

    /**
     * Memory access flags.
     *
     * The actual values of this enum depend on the architecture
     * specific constants of the virtual memory page table implementation.
     */
    typedef enum Access
    {
        None       = 0,
        Present    = 1 << 0,
        Readable   = 1 << 1,
        Writable   = 1 << 2,
        Executable = 1 << 3,
        User       = 1 << 4,
        IO         = 1 << 5
    }
    Access;

    /**
     * Memory regions.
     *
     * Various memory regions which are available on the system.
     * Memory regions are labels which refer to predefined virtual memory ranges.
     * The virtual memory address range of each region depends on the
     * architecture implementation and can be obtained with range().
     *
     * @see range
     */
    typedef enum Region
    {
        KernelData,    /**<< Kernel program data from libexec, e.g. code, bss, (ro)data */
        KernelHeap,    /**<< Kernel heap */
        KernelStack,   /**<< Kernel stack */
        KernelPrivate, /**<< Kernel dynamic memory mappings */

        PageTables,    /**<< Page table mappings */

        UserData,      /**<< User program data from libexec, e.g. code, bss, (ro)data */
        UserHeap,      /**<< User heap */
        UserStack,     /**<< User stack */
        UserPrivate,   /**<< User private dynamic memory mappings */
        UserShared     /**<< User shared dynamic memory mappings */
    }
    Region;

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
     * @param phys Physical memory allocator.
     */
    Memory(Address pageDirectory, BitAllocator *phys);

    /**
     * Destructor
     */
    virtual ~Memory();

    /**
     * Get region addresses.
     *
     * @param region Memory region.
     * @return Range object
     */
    virtual Range range(Region region) = 0;

    /**
     * Create an address space.
     *
     * @return Result code.
     */
    virtual Result create() = 0;

    /**
     * Enables the MMU.
     *
     * @return Result code.
     */
    virtual Result initialize() = 0;

    /**
     * Map a physical page to a virtual address.
     *
     * @param phys Physical address.
     * @param virt Virtual address or ZERO to use the first unused page.
     * @param access Page entry protection flags.
     * @return Result code.
     */     
    virtual Result map(Address phys, Address virt, Access access) = 0;

    /**
     * Map a range of physical pages to virtual addresses.
     *
     * @param range Range object describing the range of physical pages.
     * @return Result code.
     */
    virtual Result mapRange(Range *range);

    /**
     * Map virtual memory in a region.
     *
     * This function will allocate a certain amount
     * of pages in the physical memory allocator and map
     * them in unused virtual memory inside the given region.
     *
     * @param region Memory region to map in.
     * @param size Number of bytes to map in the region.
     * @return First virtual address of the mapping.
     */
    // TODO: should regions also define its (maximum) access permissions?
    virtual Address mapRegion(Memory::Region region, Size size, Access access);

    /**
     * Unmap a virtual address.
     *
     * This function removes a virtual to physical memory
     * mapping without deallocating any physical memory.
     *
     * @param virt Virtual address to unmap.
     * @return Result code
     */
    virtual Result unmap(Address virt) = 0;

    /**
     * Unmaps a range of virtual memory.
     *
     * @param range Range object describing the range of virtual addresses.
     * @return Result code
     */
    virtual Result unmapRange(Range *range);

    /**
     * Translate virtual address to physical address.
     *
     * @param virt Virtual address to lookup.
     * @return Physical address of the virtual address given.
     */
    virtual Address lookup(Address virt) = 0;

    /**
     * Get Access flags for a virtual address.
     *
     * @param virt Virtual address to get Access flags for.
     * @return Access flags.
     */
    virtual Access access(Address virt) = 0;

    /**
     * Release a memory page mapping.
     *
     * @param virt Virtual address of the page to release.
     * @return Result code
     */
    virtual Result release(Address virt) = 0;

    /**
     * Release a range of physical memory by its virtual memory pages.
     *
     * @param range Range object describing the range of physical pages to release.
     * @return Result code
     */
    virtual Result releaseRange(Range *range);

    /**
     * Release memory region.
     *
     * Deallocate all associated physical memory
     * which resides in the given memory region.
     *
     * @param region Memory region to release
     * @return Result code
     */
    virtual Result releaseRegion(Region region) = 0;

    /**
     * Find unused memory.
     *
     * This function finds a contigeous block of a given size
     * of virtual memory which is unused and then returns
     * the virtual address of the first page in the block.
     *
     * @param region Memory region to search in.
     * @param size Number of bytes requested to be free.
     * @return Virtual address of the first page.
     */
    virtual Address findFree(Size size, Region region) = 0;

  protected:

    /** Physical memory allocator */
    BitAllocator *m_phys;
};

#endif /* __LIBARCH_MEMORY_H */
