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

#include <Arch/Memory.h>
#include "PoolAllocator.h"
#include <MemoryBlock.h>

PoolAllocator::PoolAllocator()
{
    MemoryBlock::set(pools, 0, sizeof(pools));
}

Address PoolAllocator::allocate(Size *size)
{
    Size index, nPools = 1;
    MemoryPool *pool = ZERO;
    
    /* Find the correct pool size. */
    for (index = POOL_MIN_POWER; index < POOL_MAX_POWER; index++)
    {
	if (*size <= (Size) 1 << (index + 1)) break;
    }
    /* Do we need to allocate an initial pool? */
    if (!pools[index] && parent)
    {
	pool = pools[index] = newPool(index, POOL_MIN_COUNT(*size));
    }
    /* Search for pool with enough memory. */
    else
    {
	/* Loop current pools. */
	for (pool = pools[index]; pool; pool = pool->next, nPools++)
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
    return pool ? pool->allocate() : ZERO;
}

MemoryPool * PoolAllocator::newPool(Size index, Size cnt)
{
    MemoryPool *pool;
    Size sz;
    
    /* Prepare amount to allocate from parent. */
    sz  = cnt * (1 << (index + 1));
    sz += sizeof(MemoryPool);
    sz += BITMAP_NUM_BYTES(cnt);
    sz += MEMALIGN;

    /* Ask parent for memory, then fill in the pool. */
    if ((pool = (MemoryPool *) parent->allocate(&sz)))
    {
	pool->count  = cnt;
        pool->addr   = aligned( ((Address) (pool + 1)) + BITMAP_NUM_BYTES(pool->count) );
        pool->next   = pools[index];
        pool->free   = pool->count;
	pool->size   = (1 << (index + 1));
	pools[index] = pool;
	MemoryBlock::set(pool->blocks, 0, BITMAP_NUM_BYTES(pool->count));
    }
    return pool;
}

void PoolAllocator::release(Address addr)
{
    for (Size i = POOL_MIN_POWER - 1; i < POOL_MAX_POWER; i++)
    {
	for (MemoryPool *p = pools[i]; p; p = p->next)
	{
	    if (addr < p->addr + (p->count * p->size) &&
	        addr >= p->addr)
	    {
		p->release(addr);
		return;
	    }
	}
    }
}
