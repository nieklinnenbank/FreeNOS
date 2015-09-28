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

#include <FreeNOS/System.h>
#include "PoolAllocator.h"
#include <MemoryBlock.h>

PoolAllocator::PoolAllocator()
{
    MemoryBlock::set(m_pools, 0, sizeof(m_pools));
}

Size PoolAllocator::size()
{
    return m_parent ? m_parent->size() : ZERO;
}

Size PoolAllocator::available()
{
    return m_parent ? m_parent->available() : ZERO;
}

Allocator::Result PoolAllocator::allocate(Size *size, Address *addr, Size align)
{
    Size index, nPools = 1;
    MemoryPool *pool = ZERO;
    
    /* Find the correct pool size. */
    for (index = POOL_MIN_POWER; index < POOL_MAX_POWER; index++)
    {
        if (*size <= (Size) 1 << (index + 1)) break;
    }
    /* Do we need to allocate an initial pool? */
    if (!m_pools[index] && m_parent)
    {
        pool = m_pools[index] = newPool(index, POOL_MIN_COUNT(*size));
    }
    /* Search for pool with enough memory. */
    else
    {
        /* Loop current pools. */
        for (pool = m_pools[index]; pool; pool = pool->next, nPools++)
        {
            /* At least one block still free? */
            if (pool->free)
                break;

            /* If no pool has free space anymore, allocate another. */
            if (!pool->next)
            {
                pool = newPool(index, POOL_MIN_COUNT(*size) * nPools);
                break;
            }
        }
    }
    /* Attempt to allocate. */
    if (pool)
    {
        *addr = pool->allocate();
        return Success;
    }
    *addr = ZERO;
    return OutOfMemory;
}

MemoryPool * PoolAllocator::newPool(Size index, Size cnt)
{
    MemoryPool *pool = 0;
    Size sz;
    
    /* Prepare amount to allocate from m_parent. */
    sz  = cnt * (1 << (index + 1));
    sz += sizeof(MemoryPool);
    sz += BITMAP_NUM_BYTES(cnt);
    sz += MEMALIGN;

    /* Ask m_parent for memory, then fill in the pool. */
    if (m_parent->allocate(&sz, (Address *)&pool) == Success)
    {
        pool->count  = cnt;
        pool->addr   = aligned( ((Address) (pool + 1)) + BITMAP_NUM_BYTES(pool->count), MEMALIGN );
        pool->next   = m_pools[index];
        pool->free   = pool->count;
        pool->size   = (1 << (index + 1));
        m_pools[index] = pool;
        MemoryBlock::set(pool->blocks, 0, BITMAP_NUM_BYTES(pool->count));
    }
    return pool;
}

Allocator::Result PoolAllocator::release(Address addr)
{
    // TODO: very slow! requires a full scan of the heap in the worst case.
    for (Size i = POOL_MIN_POWER - 1; i < POOL_MAX_POWER; i++)
    {
        for (MemoryPool *p = m_pools[i]; p; p = p->next)
        {
            if (addr < p->addr + (p->count * p->size) &&
                addr >= p->addr)
            {
                p->release(addr);
                return Success;
            }
        }
    }
    return InvalidAddress;
}
