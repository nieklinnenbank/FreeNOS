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

#ifndef __LIBARCH_INTEL_PAGING_H
#define __LIBARCH_INTEL_PAGING_H

#include <Types.h>
#include "Memory.h"

/**
 * Entry inside the page directory of a given virtual address.
 * @param vaddr Virtual Address.
 * @return Index of the corresponding page directory entry.
 */
#define DIRENTRY(vaddr) \
    ((vaddr) >> DIRSHIFT)

/**
 * Entry inside the page table of a given virtual address.
 * @param vaddr Virtual Address.
 * @return Index of the corresponding page table entry.
 */
#define TABENTRY(vaddr) \
    (((vaddr) >> PAGESHIFT) & 0x3ff)

/**
 * Intel virtual memory implementation.
 */
class IntelPaging : public Memory
{
  public:

    /**
     * Constructor.
     *
     * @param pageDirectory Physical memory address of the page directory
     *                      or ZERO to map the page directory of the current
     *                      active address space.
     * @param phys          BitAllocator pointer of the physical memory page allocations
     *                      or ZERO to ask the kernel for the BitAllocator.
     */
    IntelPaging(Address pageDirectory = ZERO,
                BitAllocator *phys = ZERO);

    /**
     * Destructor.
     */
    virtual ~IntelPaging();

    /**
     * Create an address space.
     *
     * @return Result code.
     */
    virtual Result create();

    /**
     * Enables the MMU.
     *
     * @return Result code.
     */
    virtual Result initialize();

    /**
     * Get region addresses.
     *
     * @param region Memory region.
     * @return Range object
     */
    virtual Range range(Region region);

    /**
     * Map a physical page to a virtual address.
     *
     * @param paddr Physical address.
     * @param vaddr Virtual address or ZERO to use the first unused page.
     * @param prot Page entry protection flags.
     * @return Result code
     */     
    virtual Result map(Address phys, Address virt, Access flags);

    /**
     * Unmap a virtual address.
     *
     * This function removes a virtual to physical memory
     * mapping without deallocating any physical memory.
     *
     * @param virt Virtual address to unmap.
     * @return Result code
     */
    virtual Result unmap(Address virt);

    /**
     * Translate virtual address to physical address.
     *
     * @param virt Virtual address to lookup.
     * @return Physical address of the virtual address given.
     */
    virtual Address lookup(Address virt);

    /**
     * Get Access flags for a virtual address.
     *
     * @param virt Virtual address to get Access flags for.
     * @return Access flags.
     */
    virtual Access access(Address virt);

    /**
     * Release a memory page mapping.
     *
     * @param virt Virtual address of the page to release.
     * @return Result code
     */
    virtual Result release(Address virt);

    /**
     * Release memory region.
     *
     * Deallocate all associated physical memory
     * which resides in the given memory region.
     *
     * @return Result code
     */
    virtual Result releaseRegion(Region region);

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
    virtual Address findFree(Size size, Region region);

  private:

    /**
     * Get page table virtual address.
     *
     * This function will retrieve a pointer to the page table
     * which maps the given virtual address.
     *
     * @param virt The virtual address to get the page table for.
     * @return Pointer to page table in virtual address.
     */
    Address * getPageTable(Address virt);

    /**
     * Convert Memory::Access to Intel Page Entry flags.
     *
     * @param acc Memory::Access flags
     * @return Intel Page Entry flags
     */
    u32 flags(Access acc);

    /** Pointer to page directory in virtual memory. */
    Address *m_pageDirectory;

    /** Page tables virtual base address */
    Address m_pageTableBase;
};

namespace Arch
{
    typedef IntelPaging Memory;
};

#endif /* __LIBARCH_INTEL_PAGING_H */
