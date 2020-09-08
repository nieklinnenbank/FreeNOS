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

#include <Assert.h>
#include <Macros.h>
#include <MemoryBlock.h>
#include "PoolAllocator.h"

PoolAllocator::PoolAllocator(Allocator *parent)
{
    assert(parent != NULL);
    setParent(parent);
    MemoryBlock::set(m_pools, 0, sizeof(m_pools));
}

Size PoolAllocator::size() const
{
    Size totalSize, totalUsed;
    calculateUsage(totalSize, totalUsed);

    return totalSize;
}

Size PoolAllocator::available() const
{
    Size totalSize, totalUsed;
    calculateUsage(totalSize, totalUsed);

    assert(totalUsed <= totalSize);

    return totalSize - totalUsed;
}

Size PoolAllocator::calculateObjectSize(const Size index) const
{
    assert(index >= MinimumPoolSize);
    assert(index <= MaximumPoolSize);

    return 1U << index;
}

Size PoolAllocator::calculateObjectCount(const Size objectSize) const
{
    assert(objectSize > 0);
    assert(isPowerOfTwo(objectSize));

    if (objectSize >= KiloByte(16))
        return 1;
    else
        return KiloByte(16) / objectSize;
}

void PoolAllocator::calculateUsage(Size & totalSize, Size & totalUsed) const
{
    totalSize = 0;
    totalUsed = 0;

    for (Size index = MinimumPoolSize; index <= MaximumPoolSize; index++)
    {
        for (Pool *pool = m_pools[index]; pool != NULL; pool = pool->next)
        {
            totalSize += sizeof(Pool);
            totalSize += pool->bitmapSize;
            totalSize += pool->size();

            totalUsed += sizeof(Pool);
            totalUsed += pool->bitmapSize;
            totalUsed += (pool->size() - pool->available());
        }
    }
}

Allocator::Result PoolAllocator::allocate(Allocator::Range & args)
{
    const Size inputSize = aligned(args.size, sizeof(u32));
    Pool *pool = ZERO;

    // Verify input arguments
    if (args.alignment != 0)
    {
        return InvalidAlignment;
    }
    else if (inputSize == 0)
    {
        return InvalidSize;
    }

    // Find the proper pool first
    pool = retrievePool(inputSize);

    // Attempt to allocate
    if (pool)
    {
        Result result = static_cast<Allocator *>(pool)->allocate(args);
        if (result == Success)
        {
            ObjectPrefix *prefix = (ObjectPrefix *) args.address;
            prefix->signature = ObjectSignature;
            prefix->pool = pool;

            ObjectPostfix *postfix = (ObjectPostfix *) (args.address + sizeof(ObjectPrefix) + inputSize);
            postfix->signature = ObjectSignature;

            args.address += sizeof(ObjectPrefix);
        }

        return result;
    }
    else
    {
        args.address = 0;
        return OutOfMemory;
    }
}

Allocator::Result PoolAllocator::release(const Address addr)
{
    const Address actualAddr = addr - sizeof(ObjectPrefix);
    const ObjectPrefix *prefix = (const ObjectPrefix *) (actualAddr);
    ObjectPostfix *postfix = ZERO;

    // Verify the object prefix signature
    assert(prefix->signature == ObjectSignature);
    assert(prefix->pool != NULL);

    // Do a reverse memory scan to find the object postfix.
    for (Size i = prefix->pool->chunkSize() - sizeof(u32); i > sizeof(ObjectPrefix); i -= sizeof(u32))
    {
        postfix = (ObjectPostfix *)(actualAddr + i);
        if (postfix->signature == ObjectSignature)
            break;
    }

    // Verify the object postfix signature
    assert(postfix != ZERO);
    assert(postfix->signature == ObjectSignature);

    // Release the object
    Result result = prefix->pool->release(actualAddr);
    assert(result == Success);

    // Also try to release the pool itself, if no longer used
    if (prefix->pool->available() == prefix->pool->size())
    {
        releasePool(prefix->pool);
    }

    return result;
}

PoolAllocator::Pool * PoolAllocator::retrievePool(const Size inputSize)
{
    const Size requestedSize = inputSize + sizeof(ObjectPrefix) + sizeof(ObjectPostfix);
    Size index, nPools = 1, objectSize = 0;
    Pool *pool = ZERO;

    // Find the correct pool index
    for (index = MinimumPoolSize; index <= MaximumPoolSize; index++)
    {
        objectSize = calculateObjectSize(index);

        if (requestedSize <= objectSize)
            break;
    }

    // Handle too large object requests
    if (requestedSize > objectSize)
    {
        return ZERO;
    }

    // Do we need to allocate an initial pool?
    if (!m_pools[index])
    {
        pool = m_pools[index] = allocatePool(index, calculateObjectCount(objectSize));
    }
    // Search for existing pool with enough memory
    else
    {
        for (pool = m_pools[index]; pool; pool = pool->next, nPools++)
        {
            // Use current pool or if out of space allocate another
            if (pool->available())
            {
                break;
            }
            else if (!pool->next)
            {
                pool = allocatePool(index, calculateObjectCount(objectSize) * nPools);
                break;
            }
        }
    }

    return pool;
}

PoolAllocator::Pool * PoolAllocator::allocatePool(const Size index, const Size objectCount)
{
    const Size objectSize = calculateObjectSize(index);
    const Size requestBitmapSize = objectCount;
    const Size requestPayloadSize = objectCount * objectSize;
    const Size requestTotalSize = aligned(sizeof(Pool) + requestBitmapSize + requestPayloadSize, sizeof(u32));
    Pool *pool = 0;
    Allocator::Range alloc_args;

    // Allocate single buffer to store the Pool and its payload
    alloc_args.address = 0;
    alloc_args.alignment = sizeof(u32);
    alloc_args.size = requestTotalSize;

    if (parent()->allocate(alloc_args) != Allocator::Success)
    {
        return ZERO;
    }

    // The parent must have given us minimum the requested size
    assert(alloc_args.size >= requestTotalSize);

    // The parent might have returned more space than requested.
    // Calculate the optimum usage of the full space.
    Size actualObjectCount = (alloc_args.size - sizeof(Pool) - requestBitmapSize) / objectSize;
    Size actualPayloadSize = 0;
    Size actualBitmapSize = 0;
    Size actualTotalSize = 0;

    assert(actualObjectCount >= objectCount);

    while (actualObjectCount >= objectCount)
    {
        actualPayloadSize = actualObjectCount * objectSize;
        actualBitmapSize = aligned(actualObjectCount, sizeof(u32));
        actualTotalSize = sizeof(Pool) + actualBitmapSize + actualPayloadSize;

        if (actualTotalSize <= alloc_args.size)
            break;
        else
            actualObjectCount--;
    }

    // Calculate inputs for Pool object
    const Address bitmapAddr = alloc_args.address + sizeof(Pool);
    const Allocator::Range range = { bitmapAddr + actualBitmapSize, actualPayloadSize, sizeof(u32) };

    // Instantiate the Pool object
    pool = new (alloc_args.address) Pool(range, objectSize, actualBitmapSize, (u8 *) bitmapAddr);
    pool->index = index;
    pool->prev = ZERO;
    pool->next = m_pools[index];
    m_pools[index] = pool;

    if (pool->next != NULL)
        pool->next->prev = pool;

    return pool;
}

Allocator::Result PoolAllocator::releasePool(Pool *pool)
{
    Pool *prevPool = pool->prev;
    Pool *nextPool = pool->next;
    const Size index = pool->index;
    const Result parentResult = parent()->release((Address) pool);

    // Only update Pool administration if memory was released at parent
    if (parentResult == Success)
    {
        if (prevPool != NULL)
        {
            prevPool->next = nextPool;
        }

        if (nextPool != NULL)
        {
            nextPool->prev = prevPool;
        }

        if (m_pools[index] == pool)
        {
            m_pools[index] = nextPool;
        }
    }

    return parentResult;
}
