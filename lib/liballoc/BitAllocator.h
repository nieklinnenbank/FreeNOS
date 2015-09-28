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

#ifndef __LIBALLOC_BITALLOCATOR_H
#define __LIBALLOC_BITALLOCATOR_H

#include <BitArray.h>
#include <Types.h>
#include <Memory.h>
#include "Allocator.h"

/** 
 * @defgroup kernel kernel (generic)
 * @{ 
 */

/**
 * Bit memory allocator.
 *
 * Allocates memory by using a BitArray. All memory is divided
 * in same sized parts called chunks. 1 in the BitArray means
 * the chunk is used, 0 means unused.
 */
class BitAllocator : public Allocator
{
  public:

    /**
     * Constructor function.
     *
     * @param range Contigeous range of memory to manage.
     * @param chunkSize Total memory will be divided into chunks.
     */
    BitAllocator(Memory::Range range, Size chunkSize);

    /**
     * Get chunk size.
     *
     * @return Chunk size.
     */
    Size chunkSize();

    /**
     * Get total size.
     *
     * @return Total size.
     */
    virtual Size size();

    /**
     * Get available memory.
     *
     * @return Available memory.
     */
    virtual Size available();

    /**
     * Get base memory address.
     */
    Address base();

    /**
     * Get allocation BitArray.
     *
     * @return BitArray object pointer.
     */
    BitArray * getBitArray();

    /**
     * Allocate memory.
     *
     * @param size Size of memory to allocate.
     * @param addr Address allocated.
     * @param align Alignment of the required memory or use
     *              ZERO for chunksize. Must be a multiple
     *              of the chunksize.
     * @return Result value.
     */
    virtual Result allocate(Size *size, Address *addr, Size align = ZERO);

    /**
     * Allocate memory.
     *
     * @param size Size of memory to allocate.
     * @param addr Address allocated.
     * @param align Alignment of the required memory or use
     *              ZERO for chunksize. Must be a multiple
     *              of the chunksize.
     * @param allocStart Allocation address to start searching at.
     * @return Result value.
     */
    Result allocate(Size *size, Address *addr, Size align = ZERO, Address allocStart = 0);

    /**
     * Allocate address.
     *
     * @param addr Allocate a specific address.
     * @return Result value.
     */
    Result allocate(Address addr);

    /**
     * Check if a chunk is allocated.
     *
     * @return True if allocated, false otherwise.
     */
    bool isAllocated(Address page);

    /**
     * Release memory chunk.
     *
     * @param chunk The memory chunk to release.
     * @return Result value.
     */
    virtual Result release(Address chunk);

  private:

    /** Marks which chunks are (un)used. */
    BitArray m_array;

    /** Start of memory region. */
    Address m_base;

    /** Size of each chunk. */
    Size m_chunkSize;
};

/**
 * @}
 */

#endif /* __MEMORY_H */
