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

#ifndef __ARM_ARMMEMORY_H
#define __ARM_ARMMEMORY_H

#include <FreeNOS/Memory.h>
#include <Types.h>
#include <Macros.h>
#include "ARMProcess.h"

/** ARM uses 4K pages. */
#define PAGESIZE        4096

/** Memory address alignment. */
#define MEMALIGN        4

/** Marks a page entry present. */
#define PAGE_PRESENT    1

/** Marks a page entry read/write. */
#define PAGE_RW         1

/** Marks a page accessible by user programs. */
#define PAGE_USER       1

/** Pinned pages cannot be released. */
#define PAGE_PINNED     1

/** This page has been marked for temporary operations. */
#define PAGE_MARKED     1

/** Page has been reserved for future use. */
#define PAGE_RESERVED   1

/**
 * Convert MemoryAccess to ARM specific page protection flags.
 * @return ARM page protection flags.
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

class ARMMemory : public Memory
{
  public:

    /**
     * Constructor function.
     */
    ARMMemory(Size memorySize);

    /**
     * Map a physical page to a virtual address.
     *
     * @param paddr Physical address.
     * @param vaddr Virtual address.
     * @param prot Page entry protection flags.
     * @return Mapped virtual address.
     */     
    virtual Address map(Address paddr,
                        Address vaddr = ZERO,
                        MemoryAccess flags = Memory::Present | Memory::Readable | Memory::Writable);

    /**
     * Map a physical page to a virtual address for a specific Process.
     *
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
     *
     * @param p Target process.
     * @param vaddr Virtual address to lookup.
     * @return Page table entry if vaddr is mapped, or ZERO if not.
     */
    virtual Address lookup(Process *p, Address vaddr);

    /**
     * Verify protection access flags in the page directory and page table.
     *
     * @param p Target process to verify protection bits for.
     * @param vaddr Virtual address.
     * @param sz Size of the byte range to check.
     * @return True if the current process has access, false otherwise.
     */
    virtual bool access(Process *p,
                        Address vaddr,
                        Size sz,
                        MemoryAccess flags = Memory::Present|Memory::User|Memory::Readable);

    /** 
     * Marks all physical pages used by a process as free (if not pinned). 
     *
     * @param p Target process. 
     */
    virtual void release(Process *p);

  private:

};

#endif /* __ARM_ARMMEMORY_H */
