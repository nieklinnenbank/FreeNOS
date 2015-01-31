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

#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#ifndef __ASSEMBLER__

#include <Types.h>

/** 
 * @defgroup kernel kernel (generic)
 * @{ 
 */

/** Forward declaration. */
class Process;

/**
 * Represents system memory.
 */
class Memory
{
    public:

        /**
         * Constructor function.
         */
        Memory();

        /**
         * Get the total amount of physical memory.
         * @return Amount of physical memory, in bytes.
         */
        Size getTotalMemory();
        
        /**
         * Retrieve available physical memory, in bytes.
         * @return Amount of memory available (not used) in bytes.
         */
        Size getAvailableMemory();

        /**
         * Setup the memoryMap and kernel heap.
         */
        static void initialize();

        /**
         * Allocates and marks physical memory used in the memoryMap.
         * @param sz Amount of memory to allocate.
         * @param addr Physical address to start searching at.
         * @return Physical address of the allocated memory.
         */
        Address allocatePhysical(Size sz, Address addr = 4194304);

        /**
         * Unmarks physical memory used in the memoryMap.
         * @param paddr Physical address of the memory to unmark.
         */
        void releasePhysical(Address paddr);

        /**
         * Check if a physical memory page is marked.
         * @param addr Physical address to check.
         */
        bool isMarked(Address paddr);

        /**
         * Allocate a new virtual memory page.
         * @param vaddr Virtual address.
         * @param prot Page protection flags.
         * @return Allocated physical address.
         */
        Address allocateVirtual(Address vaddr, ulong prot);

        /**
         * Allocate a new virtual memory page.
         * @param p Process to allocate memory for.
         * @param vaddr Virtual address.
         * @param prot Page protection flags.
         * @return Allocated physical address.
         */
        Address allocateVirtual(Process *p, Address vaddr, ulong prot);

        /**
         * Map a physical page into the virtual address space of a Process.
         * @param paddr Physical address.
         * @param vaddr Virtual address.
         * @param prot Protection flags.
         * @return The mapped virtual address.
         */
        virtual Address mapVirtual(Address paddr, Address vaddr, ulong prot) = 0;

        /**
         * Map a physical page into the virtual address space of a Process.
         * @param p Process for which we map memory.
         * @param paddr Physical address.
         * @param vaddr Virtual address.
         * @param prot Protection flags.
         * @return The mapped virtual address.
         */
        virtual Address mapVirtual(Process *p, Address paddr,
                                   Address vaddr, ulong prot) = 0;

        /**
         * Lookup a pagetable entry for the given (remote) virtual address.
         * @param p Target process.
         * @param vaddr Virtual address to lookup.
         * @return Page table entry if vaddr is mapped, or ZERO if not.
         */
        virtual Address lookupVirtual(Process *p, Address vaddr) = 0;

        /** 
         * Verify protection access flags.
         * @param p Target process.
         * @param vaddr Virtual address. 
         * @param sz Size of the byte range to check. 
         * @return True if the current process has access, false otherwise. 
         */
        virtual bool access(Process *p, Address vaddr, Size sz,
                            ulong prot = PAGE_PRESENT|PAGE_RW|PAGE_USER) = 0;

        /**
         * Marks all physical pages used by a process as free (if not pinned).
         * @param p Target process.
         */
        virtual void releaseAll(Process *p) = 0;

    protected:

        /** Total and available amount of memory. */
        static Size memorySize, memoryAvail;
        
        /** Maps all available memory. */
        static u8 *memoryMap, *memoryMapEnd;

    private:

        /**
         * (Un)mark a physical page.
         * @param addr Physical address to be (un)marked.
         * @param marked Either marks or unmarks the page.
         */
        void setMark(Address addr, bool marked);
};

/**
 * @}
 */

#endif /* __ASSEMBLY__ */
#endif /* __MEMORY_H */
