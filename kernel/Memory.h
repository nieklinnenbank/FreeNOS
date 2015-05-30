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

    /**
     * Constructor function.
     * @param memorySize Total amount of memory pages.
     * @param kernelAddress Address of the kernel in physical memory.
     * @param kernelSize Size of the kernel.
     */
    // TODO: this should be a memory map instead? We cannot assue there is
    // always a contigeous block of memory available.
    Memory(Size size);

    /**
     * Initialize Heap.
     *
     * @param heap Virtual start address of the kernel heap.
     * @param size Size of the heap in bytes.
     * @return Error code.
     */
    static Error initialize(Address heap, Size size);

    /**
     * Retrieve total system memory.
     */
    Size getTotalMemory();

    /**
     * Retrieve unused system memory.
     */
    Size getAvailableMemory();

    /**
     * Get the physical memory allocation bitmap.
     */
    BitArray * getMemoryBitArray();

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

  private:

    /** Physical memory BitArray. */
    BitArray m_physicalMemory;
};

/**
 * @}
 */

#endif /* __MEMORY_H */
