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

#ifndef __LIBARCH_INTEL_MEMORY_H
#define __LIBARCH_INTEL_MEMORY_H

#include <Types.h>
#include "VirtualMemory.h"

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
class IntelMemory : public VirtualMemory
{
  public:

    /**
     * Constructor.
     *
     * @param pageDirectory Physical memory address of the page directory
     *                      or ZERO to map the page directory of the current
     *                      active address space.
     * @param memoryMap     BitArray pointer of the physical memory page allocations
     *                      or ZERO to ask the kernel for the BitArray.
     */
    IntelMemory(Address pageDirectory = ZERO,
                BitArray *memoryMap = ZERO);

    /**
     * Destructor.
     */
    virtual ~IntelMemory();

    /**
     * Map a physical page to a virtual address.
     *
     * @param paddr Physical address.
     * @param vaddr Virtual address or ZERO to use the first unused page.
     * @param prot Page entry protection flags.
     * @return Mapped virtual address.
     */     
    virtual Address map(Address phys,
                        Address virt = ZERO,
                        Access flags = Present | User | Readable | Writable);

    /**
     * Unmap a virtual address.
     *
     * This function removes a virtual to physical memory
     * mapping without deallocating any physical memory.
     *
     * @param virt Virtual address to unmap.
     */
    virtual void unmap(Address virt);

    /**
     * Translate virtual address to physical address.
     *
     * @param virt Virtual address to lookup.
     * @return Physical address of the virtual address given.
     */
    virtual Address lookup(Address virt);

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
                        Access flags = Present | User | Readable);

    /**
     * Clone a virtual memory address space.
     *
     * Clones the given virtual memory address space
     * into this VirtualMemory.
     *
     * @param pageDirectory Physical address of the page directory to clone.
     * @return True if clone with success, false otherwise.
     */
    virtual bool clone(Address pageDirectory);

    /**
     * Release a memory page mapping.
     *
     * @param virt Virtual address of the page to release.
     */
    virtual void release(Address virt);

    /** 
     * Deallocate all associated physical memory.
     */
    virtual void releaseAll();

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
    virtual Address findFree(Size size);

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

    /** Pointer to page directory in virtual memory. */
    Address *m_pageDirectory;

    /** Page tables virtual base address */
    Address m_pageTableBase;
};

namespace Arch
{
    typedef IntelMemory Memory;
};

#endif /* __LIBARCH_INTEL_MEMORY_H */
