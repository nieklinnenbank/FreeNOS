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

#ifndef __LIBALLOC_POOLALLOCATOR_H
#define __LIBALLOC_POOLALLOCATOR_H

#include <Types.h>
#include <Macros.h>
#include "Allocator.h"
#include "BitAllocator.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup liballoc
 * @{
 */

/**
 * Memory allocator which uses pools that each manage same-sized objects.
 *
 * Allocates memory from pools each having the size of a power of two.
 * Each pool is pre-allocated and has a bitmap representing free blocks.
 *
 * @todo While this Allocator works well, its performance can be improved. The BitAllocator
 *       contains a BitArray that scans its internal array for "free bits". If the caller
 *       is unfortunate, the whole array needs to be scanned, adding overhead. The Linux kernel
 *       uses a buddy allocator, that basically combines a bit array and a linked list for optimal performance.
 */
class PoolAllocator : public Allocator
{
  private:

    /** Minimum power of two for a pool size. */
    static const Size MinimumPoolSize = 2;

    /** Maximum power of two size a pool can be (128MiB). */
    static const Size MaximumPoolSize = 27;

    /** Signature value is used to detect object corruption/overflows */
    static const u32 ObjectSignature = 0xF7312A56;

    /**
     * Allocates same-sized objects from a contiguous block of memory.
     */
    typedef struct Pool : public BitAllocator
    {
        Pool(const Range & range,
             const Size objectSize,
             const Size bitmapSize,
             u8 *bitmap)
        : BitAllocator(range, objectSize, bitmap)
        , prev(ZERO)
        , next(ZERO)
        , index(0)
        , bitmapSize(bitmapSize)
        {
        }

        Pool *prev;            /**< Points to the previous pool of this size (if any). */
        Pool *next;            /**< Points to the next pool of this size (if any). */
        Size index;            /**< Index number in the m_pools array where this Pool is stored. */
        const Size bitmapSize; /**< Size in bytes of the bitmap array. */
    } Pool;

    /**
     * This data structure is prepended in memory before each object
     */
    typedef struct ObjectPrefix
    {
        u32 signature;  /**< Filled with a fixed value to detect corruption/overflows */
        Pool *pool;     /**< Points to the Pool instance where this object belongs to */
    } ObjectPrefix;

    /**
     * Appended in memory after each object
     */
    typedef struct ObjectPostfix
    {
        u32 signature;  /**< Filled with a fixed value to detect corruption/overflows */
    } ObjectPostfix;

  public:

    /**
     * Constructor
     *
     * @param parent Allocator for obtaining new memory to manage
     */
    PoolAllocator(Allocator *parent);

    /**
     * Get memory size.
     *
     * @return Size of memory owned by the PoolAllocator.
     */
    virtual Size size() const;

    /**
     * Get memory available.
     *
     * @return Size of memory available by the PoolAllocator.
     */
    virtual Size available() const;

    /**
     * Allocate memory.
     *
     * @param args Contains the requested size and alignment on input.
     *             On output, contains the actual allocated address.
     *
     * @return Result value.
     */
    virtual Result allocate(Range & args);

    /**
     * Release memory.
     *
     * @param addr Points to memory previously returned by allocate().
     *
     * @return Result value.
     *
     * @see allocate
     */
    virtual Result release(const Address addr);

  private:

    /**
     * Calculate object size given the Pool index number.
     *
     * @param index Index number in m_pools
     *
     * @return Size in bytes
     */
    Size calculateObjectSize(const Size index) const;

    /**
     * Calculate minimum object count for a Pool.
     *
     * @param objectSize Size per-object in bytes
     *
     * @return Minimum number of objects to allocate
     */
    Size calculateObjectCount(const Size objectSize) const;

    /**
     * Determine total memory usage.
     *
     * @param totalSize Total memory in bytes owned by the PoolAllocator.
     * @param totalUsed Total memory in bytes actually used.
     */
    void calculateUsage(Size & totalSize, Size & totalUsed) const;

    /**
     * Find a Pool of sufficient size.
     *
     * @param inputSize Requested size of object to store
     *
     * @return Pool object pointer on success or NULL on failure.
     */
    Pool * retrievePool(const Size inputSize);

    /**
     * Creates a new Pool instance.
     *
     * @param index Index in the Pools array
     * @param objectCount Allocate for this many of blocks from our parent.
     *
     * @return Pointer to a Pool object on success, ZERO on failure.
     */
    Pool * allocatePool(const uint index, const Size objectCount);

    /**
     * Release Pool instance memory.
     *
     * @param pool Pool object pointer
     *
     * @return Result value.
     */
    Result releasePool(Pool *pool);

  private:

    /** Array of memory pools. Index represents the power of two. */
    Pool *m_pools[MaximumPoolSize + 1];
};

/**
 * @}
 * @}
 */

#endif /* __LIBALLOC_POOLALLOCATOR_H */
