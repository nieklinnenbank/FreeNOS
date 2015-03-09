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

#ifndef __INTEL_MEMORY_H
#define __INTEL_MEMORY_H

/**   
 * @defgroup x86kernel kernel (x86)  
 * @{   
 */

#ifdef __ASSEMBLER__
#define ADDRESS
#define ADDRESSP
#else
#define ADDRESS   (Address)
#define ADDRESSP  (Address *)
#endif

/** Page entry bit shift. */
#define PAGESHIFT       12

/** Page table bit shift. */
#define DIRSHIFT        22

/** Intel uses 4K pages. */
#define PAGESIZE        4096

/** Number of entries in the page directory. */
#define PAGEDIR_MAX     1024

/** Number of entries in a page table. */
#define PAGETAB_MAX     1024

/** Mask to find the page. */
#define PAGEMASK        0xfffff000 

/** Memory address alignment. */
#define MEMALIGN        4

/** Marks a page entry present. */
#define PAGE_PRESENT    1

/** Marks a page entry read/write. */
#define PAGE_RW         2

/** Marks a page accessible by user programs (ring 3). */
#define PAGE_USER       4

/** Pinned pages cannot be released. */
#define PAGE_PINNED     (1 << 9)

/** This page has been marked for temporary operations. */
#define PAGE_MARKED     (1 << 10)

/** Page has been reserved for future use. */
#define PAGE_RESERVED   (1 << 11)

/** We map page tables into virtual memory, at a fixed address. */
#define PAGETABFROM             ADDRESS (1024 * 1024 * 4)

/** Address space for modifing remote page tables. */
#define PAGETABFROM_REMOTE      ADDRESS (1024 * 1024 * 8)

/** Address space for userspace pagetable mapping. */
#define PAGEUSERFROM            ADDRESS (1024 * 1024 * 12)

/**
 * Entry inside the page directory of a given virtual address.
 * @param vaddr Virtual Address.
 * @return Index of the corresponding page directory entry.
 */
#define DIRENTRY(vaddr) \
    ((ADDRESS vaddr) >> DIRSHIFT)

/**
 * Entry inside the page table of a given virtual address.
 * @param vaddr Virtual Address.
 * @return Index of the corresponding page table entry.
 */
#define TABENTRY(vaddr) \
    (((ADDRESS vaddr) >> PAGESHIFT) & 0x3ff)

/**
 * Compute the virtual address of the page table, for a given vaddr.
 * @param vaddr Virtual address.
 * @param from Address of the first page table.
 * @return Address pointer to the page table for vaddr.
 */
#define PAGETABADDR_FROM(vaddr,from) \
    ADDRESSP (((((vaddr) >> (PAGESHIFT)) * 4) + (from)) & PAGEMASK)

/**
 * Compute the virtual address of the page table, for a given vaddr.
 * @param vaddr Virtual address.
 * @return Address pointer of the page table for vaddr.
 */
#define PAGETABADDR(vaddr) \
    PAGETABADDR_FROM(vaddr, PAGETABFROM)

/**
 * Virtual address pointing to the page directory.
 */
#define PAGEDIRADDR \
    PAGETABADDR(PAGETABFROM)

/**
 * Virtual address pointer to the page directory.
 * @param from Address of the first page table.
 * @return Address pointer to the page directory table.
 */
#define PAGEDIRADDR_FROM(from) \
    PAGETABADDR_FROM(PAGETABFROM, from)

#define PAGEDIRADDR_REMOTE \
    PAGETABADDR_FROM(PAGETABFROM, PAGETABFROM_REMOTE)

#ifndef __ASSEMBLER__
#ifndef __HOST__

/**
 * Flushes the Translation Lookaside Buffers (TLB) for a single page.
 * @param addr Memory address to flush.
 */
#define tlb_flush(addr) \
    asm volatile("invlpg (%0)" ::"r" (addr) : "memory")

/**
 * Flushes all Translation Lookaside Buffers (TLB).
 */
#define tlb_flush_all() \
    asm volatile("mov %cr3, %eax\n" \
                 "mov %eax, %cr3\n")

#include <FreeNOS/Memory.h>
#include <Types.h>
#include <Macros.h>
#include "IntelProcess.h"

/**
 * Convert MemoryAccess to Intel specific page protection flags.
 * @return Intel page protection flags.
 */
inline ulong getProtectionFlags(Memory::MemoryAccess flags)
{
    ulong prot = ZERO;

    if (flags & Memory::Present)  prot |= PAGE_PRESENT;
    if (flags & Memory::Writable) prot |= PAGE_RW;
    if (flags & Memory::User)     prot |= PAGE_USER;
    if (flags & Memory::Pinned)   prot |= PAGE_PINNED;
    if (flags & Memory::Marked)   prot |= PAGE_MARKED;
    if (flags & Memory::Reserved) prot |= PAGE_RESERVED;

    return prot;
}

inline Memory::MemoryAccess getMemoryAccess(ulong flags)
{
    Memory::MemoryAccess access;

    if (flags & PAGE_PRESENT)     access |= Memory::Present;
    if (flags & PAGE_RW)          access |= Memory::Writable;
    if (flags & PAGE_USER)        access |= Memory::User;
    if (flags & PAGE_PINNED)      access |= Memory::Pinned;
    if (flags & PAGE_MARKED)      access |= Memory::Marked;
    if (flags & PAGE_RESERVED)    access |= Memory::Reserved;

    return access;
}

/**
 * x86 Virtual Memory.
 */
class IntelMemory : public Memory
{
    public:

        /**
         * Constructor function.
         */
        IntelMemory(Size memorySize);

        /**
         * Map a physical page to a virtual address, using intel's paged virtual memory.
         * @param paddr Physical address.
         * @param vaddr Virtual address.
         * @param prot Page entry protection flags.
         * @return Mapped virtual address.
         */     
        virtual Address map(Address paddr,
                            Address vaddr = ZERO,
                            MemoryAccess flags = Memory::Present | Memory::Readable | Memory::Writable);

        /**
         * Map a physical page to a virtual address, using intel's paged virtual memory.
         * @param p Process to map memory for.
         * @param paddr Physical address.
         * @param vaddr Virtual address.
         * @param prot Page entry protection flags.
         * @return Mapped virtual address.
         */     
        virtual Address map(Process *p, Address paddr,
                            Address vaddr, MemoryAccess flags);

        /**
         * Lookup a pagetable entry for the given (remote) virtual address.
         * @param p Target process.
         * @param vaddr Virtual address to lookup.
         * @return Page table entry if vaddr is mapped, or ZERO if not.
         */
        virtual Address lookup(Process *p, Address vaddr);

        /**
         * Verify protection access flags in the page directory and page table.
         * @param p Target process to verify protection bits for.
         * @param vaddr Virtual address.
         * @param sz Size of the byte range to check.
         * @return True if the current process has access, false otherwise.
         */
        virtual bool access(Process *p, Address vaddr, Size sz, MemoryAccess flags = Memory::Present|Memory::User|Memory::Readable);

        /** 
         * Marks all physical pages used by a process as free (if not pinned). 
         * @param p Target process. 
         */
        void release(Process *p);

        /**
         * Maps remote pages into the current process.
         * @param p Other process for which we map tables.
         * @param pageTabAddr Point page table pointer for this address.
         * @param pageDirAddr Map the remote page remote directory on this address.
         * @param prot Extra memory protection flags for the mapping.
         */
        void mapRemote(IntelProcess *p, Address pageTabaddr,
                       Address pageDirAddr = (Address) PAGEDIRADDR_REMOTE,
                       MemoryAccess = Memory::None);

    private:
    
        /**
         * Find a free virtual page.
         * @param pageTabFrom Address of the first page table.
         * @param pageDirAddr Pointer to the page directory to use.
         * @return Free virtual address, if any.
         */
        Address findFree(Address pageTabFrom, Address *pageDir);
        
        /** Remote page directory and page tables. */
        Address *m_remPageDir, *m_remPageTab;
        
        /** Local (i.e. currently executing process) page directory and tables. */
        Address *m_myPageDir, *m_myPageTab;
};

/** Kernel page directory. */
extern Address kernelPageDir[1024], kernelPageTab[1024];

#endif /* __HOST__ */
#endif /* __ASSEMBLER__ */

/**
 * @}
 */

#endif /* __INTEL_MEMORY_H */

