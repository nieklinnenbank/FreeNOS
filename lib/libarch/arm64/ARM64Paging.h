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

#ifndef __LIBARCH_ARM64_PAGING_H
#define __LIBARCH_ARM64_PAGING_H

#include <Types.h>
#include "MemoryContext.h"
#include "MemoryMap.h"

/** Forward declaration */
class SplitAllocator;
class ARM64PageTable;

//Learn the architecture - AArch64 memory attributes and properties
#define MEM_ATTR_NORMAL 0xff //normal, IWBWA, OWBWA, NTR
#define MEM_ATTR_DEV    0x04 //device, nGnRE
#define MEM_ATTR_NC     0x44 //non cacheable
#define MAIR_FIELD(val, idx) ((val)<<((idx)<<3))

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libarch
 * @{
 *
 * @addtogroup libarch_arm
 * @{
 */

/**
 * ARM64 virtual memory implementation.
 */
class ARM64Paging : public MemoryContext
{
  public:

    /**
     * Constructor.
     *
     * @param map Virtual memory map.
     * @param alloc Allocator pointer of the physical memory page allocations.
     */
    ARM64Paging(MemoryMap *map, SplitAllocator *alloc);

    /**
     * Secondary constructor with pre-allocated 1st page table.
     *
     * @param map Virtual memory map
     * @param firstTableAddress Physical address of 1st page table
     * @param kernelBaseAddress Physical base address of the kernel
     */
    ARM64Paging(MemoryMap *map, Address firstTableAddress,
                Address secondTableAddress[2], Address kernelBaseAddress);

    /**
     * Destructor.
     */
    virtual ~ARM64Paging();

    /**
     * Initialize the MemoryContext
     *
     * @return Result code
     */
    virtual Result initialize();

    /**
     * Activate the MemoryContext.
     *
     * This function applies this MemoryContext on the hardware MMU.
     *
     * @param initializeMMU If true perform (re)initialization of the MMU
     *
     * @return Result code.
     */
    virtual Result activate(bool initializeMMU = false);

    /**
     * Map a physical page to a virtual address.
     *
     * @param virt Virtual address.
     * @param phys Physical address.
     * @param access Memory access flags.
     *
     * @return Result code
     */
    virtual Result map(Address virt, Address phys, Memory::Access access);

    /**
     * Unmap a virtual address.
     *
     * This function removes a virtual to physical memory
     * mapping without deallocating any physical memory.
     *
     * @param virt Virtual address to unmap.
     *
     * @return Result code
     */
    virtual Result unmap(Address virt);

    /**
     * Translate virtual address to physical address.
     *
     * @param virt Virtual address to lookup on input
     * @param phys Contains the physical address on output.
     *
     * @return Result code
     */
    virtual Result lookup(Address virt, Address *phys) const;

    /**
     * Get Access flags for a virtual address.
     *
     * @param virt Virtual address to get Access flags for.
     * @param access MemoryAccess object pointer.
     *
     * @return Result code.
     */
    virtual Result access(Address virt, Memory::Access *access) const;

    /**
     * Release memory sections.
     *
     * Deallocate all associated physical memory
     * which resides in the given memory section range.
     *
     * @param range Range of memory sections to release
     * @param tablesOnly True to only release associated page tables
     *                   and do not release the actual mapped pages
     *
     * @return Result code
     */
    virtual Result releaseSection(const Memory::Range & range,
                                  const bool tablesOnly = false);

    /**
     * Release range of memory.
     *
     * @param range Memory range input
     *
     * @return Result code.
     */
    virtual Result releaseRange(Memory::Range *range);

  private:

    /**
     * Installs default mappings on 1st level page table
     *
     * @param map Virtual memory map
     * @param firstTableAddress Physical address of 1st level page table
     * @param kernelBaseAddress Physical address of the kernel base
     */
    void setupPageTable(MemoryMap *map, Address firstTableAddress, Address kernelBaseAddress);

    /**
     * Enable the MMU
     *
     * @return Result code
     */
    Result enableMMU();

  private:

    /** Pointer to the first level page table. */
    ARM64PageTable *m_firstTable;

    /** Physical address of the first level page table. */
    Address m_firstTableAddr;

    /** Kernel base address */
    Address m_kernelBaseAddr;
#if 0
    /** Caching implementation */
    Arch::Cache m_cache;
#endif
    /** Physical address of the second level page table. */
    Address m_secondTableAddr[2]; //FIXME: make it flexible as a macro
    enum {
        BootStage,          // in early boot
        KernelStage,        // in kernel run
    } m_pageStage;
};

namespace Arch
{
    typedef ARM64Paging Memory;
};

/**
 * @}
 * @}
 * @}
 */

#endif /* __LIBARCH_ARM64_PAGING_H */
