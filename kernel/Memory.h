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

#ifndef __MEMORY_H
#define __MEMORY_H

#include <BitArray.h>
// TODO: make sure the bit operations are only for MemoryAccess, and not in general. Use templating.
#include <BitOperations.h>
#include <Types.h>
#include "Process.h"

/** 
 * @defgroup kernel kernel (generic)
 * @{ 
 */

/**
 * Represents system memory.
 */
class Memory
{
    public:

        /** Memory access flags */
        enum MemoryAccess
        {
            None       = (1 << 0),
            Present    = (1 << 1),
            Readable   = (1 << 2),
            Writable   = (1 << 3),
            Executable = (1 << 4),
            User       = (1 << 5),
            Pinned     = (1 << 6),
            Reserved   = (1 << 7),
            Marked     = (1 << 8)
        };

        /**
         * Constructor function.
         * @param memorySize Total amount of memory pages.
         */
        // TODO: this should be a memory map instead? We cannot assue there is
        // always a contigeous block of memory available.
        Memory(Size memorySize);

        /**
         * Initialize heap at the given address.
         */
        static Error initialize(Address heap);

        /**
         * Retrieve total system memory.
         */
        Size getTotalMemory();

        /**
         * Retrieve unused system memory.
         */
        Size getAvailableMemory();

        /**
         * Allocate a new physical page.
         * @return The allocated physical page.
         */
        Address allocatePhysical(Size size);

        /**
         * Mark specific physical address used.
         */
        Address allocatePhysicalAddress(Address addr);        

        /**
         * Check if a physical page is allocated.
         */
        bool isAllocated(Address page);

        /**
         * Release a physical page.
         * @param page Address to release.
         */
        Error releasePhysical(Address page);
    
        /**
         * Allocate a new virtual memory page.
         * @param vaddr Virtual address.
         * @param prot Page protection flags.
         * @return Allocated address.
         */
        Address allocate(Address vaddr, MemoryAccess flags);

        /**
         * Allocate a new virtual memory page.
         * @param p Process to allocate memory for.
         * @param vaddr Virtual address.
         * @param prot Page protection flags.
         * @return Allocated physical address.
         */
        Address allocate(Process *p, Address vaddr, MemoryAccess flags);

        /**
         * Map a physical page into the virtual address space of a Process.
         * @param paddr Physical address.
         * @param vaddr Virtual address.
         * @param prot Protection flags.
         * @return The mapped virtual address.
         */
        virtual Address map(Address paddr,
                            Address vaddr = ZERO,
                            MemoryAccess flags = Present | Readable | Writable) = 0;

        /**
         * Map a physical page into the virtual address space of a Process.
         * @param p Process for which we map memory.
         * @param paddr Physical address.
         * @param vaddr Virtual address.
         * @param prot Protection flags.
         * @return The mapped virtual address.
         */
        virtual Address map(Process *p, Address paddr,
                            Address vaddr, MemoryAccess prot) = 0;

        // TODO: Why is there nothing to unmap a virtual memory page?

        /**
         * Lookup a pagetable entry for the given (remote) virtual address.
         * @param p Target process.
         * @param vaddr Virtual address to lookup.
         * @return Page table entry if vaddr is mapped, or ZERO if not.
         */
        virtual Address lookup(Process *p, Address vaddr) = 0;

        /** 
         * Verify protection access flags.
         * @param p Target process.
         * @param vaddr Virtual address. 
         * @param sz Size of the byte range to check. 
         * @return True if the current process has access, false otherwise. 
         */
        virtual bool access(Process *p, Address vaddr,
                            Size sz,
                            MemoryAccess flags = (Present|Readable|Writable|User)) = 0;

        /**
         * Marks all physical pages used by a process as free (if not pinned).
         * @param p Target process.
         */
        virtual void release(Process *p) = 0;

    private:

        /** Physical memory BitArray. */
        BitArray m_physicalMemory;
};

/**
 * @}
 */

#endif /* __MEMORY_H */
