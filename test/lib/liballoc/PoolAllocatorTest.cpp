/*
 * Copyright (C) 2020 Niek Linnenbank
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

#include <FreeNOS/Constant.h>
#include <TestCase.h>
#include <TestRunner.h>
#include <TestInt.h>
#include <TestMain.h>
#include <Assert.h>
#include <MemoryBlock.h>
#include <BubbleAllocator.h>
#include <PoolAllocator.h>

/**
 * Simple wrapper around the default new/delete operators.
 *
 * This construction allows the host OS to use its own
 * allocation operators, which can be validated using
 * dynamic analysis tools like valgrind.
 *
 * @see http://www.valgrind.org
 */
class DummyParent : public Allocator
{
    virtual Result allocate(Range & args)
    {
        u8 *buf = new u8[args.size];
        assert(buf != ZERO);
        MemoryBlock::set(buf, 0, args.size);
        args.address = (Address) buf;
        return args.address != ZERO ? Success : OutOfMemory;
    }

    virtual Result release(const Address addr)
    {
        delete[] (u8 *) addr;
        return Success;
    }
};

TestCase(PoolConstruct)
{
    TestInt<uint> addresses(UINT_MIN, UINT_MAX);
    const Size rangeSize = PAGESIZE * 64;
    const Allocator::Range range = { addresses.random(), rangeSize, sizeof(u32) };

    BubbleAllocator bubble(range);
    PoolAllocator pa(&bubble);

    // Verify initial state after construction
    testAssert(pa.parent() == &bubble);
    testAssert(pa.base() == ZERO);
    testAssert(pa.available() == 0);
    testAssert(pa.size() == 0);
    testAssert(pa.alignment() == 0);

    for (Size i = 0; i < PoolAllocator::MaximumPoolSize; i++)
    {
        testAssert(pa.m_pools[i] == 0);
    }

    return OK;
}

TestCase(PoolUsage)
{
    DummyParent parent;
    PoolAllocator pa(&parent);

    // Verify that the calculated total size and used bytes is correct
    testAssert(pa.size() == 0);
    testAssert(pa.available() == 0);

    Allocator::Range args = { 0, sizeof(u32), 0 };
    testAssert(pa.allocate(args) == Allocator::Success);

    testAssert(pa.size() > 0);
    testAssert(pa.available() > 0);
    testAssert(pa.available() < pa.size());

    testAssert(pa.release(args.address) == Allocator::Success);
    testAssert(pa.size() == 0);
    testAssert(pa.available() == 0);

    return OK;
}

TestCase(PoolAllocate)
{
    const Size bubbleSize = PAGESIZE * 128;
    const Address bubbleBase = (Address) (new u8[bubbleSize]);
    const Allocator::Range bubbleRange = { bubbleBase, bubbleSize, sizeof(u32) };

    BubbleAllocator bubble(bubbleRange);
    PoolAllocator pa(&bubble);

    // Allocate integers
    for (Size i = 0; i < 100; i++)
    {
        Allocator::Range args = { 0, 64, 0 };

        // Perform the allocation
        testAssert(pa.allocate(args) == Allocator::Success);
        testAssert(args.alignment == 0);
        testAssert(args.size == 64);

        // Check only the appropriate pool is created
        for (Size j = 0; j < PoolAllocator::MaximumPoolSize; j++)
        {
            if (j == 7) {
                testAssert(pa.m_pools[j] != ZERO);
            } else {
                testAssert(pa.m_pools[j] == ZERO);
            }
        }

        // Calculate the expected address
        const Address payloadBase = bubbleBase + sizeof(PoolAllocator::Pool) + pa.m_pools[7]->bitmapSize;
        const Address objectOffset = (i * pa.m_pools[7]->m_chunkSize) + sizeof(PoolAllocator::ObjectPrefix);

        // Verify the returned address is correct
        testAssert(args.address >= payloadBase);
        testAssert(args.address < bubbleBase + bubbleSize);
        testAssert(args.address == payloadBase + objectOffset);
    }

    delete[] (u8 *) bubbleBase;
    return OK;
}

TestCase(PoolRelease)
{
    const Size numObjects = 2000;
    DummyParent parent;
    PoolAllocator pa(&parent);
    Vector<u8 *> objects;
    TestInt<uint> sizes(1, 256);
    TestInt<Size> indexes(0, numObjects);

    // Randomly allocate many objects of sizes ranging from 4 bytes to 1K
    for (Size i = 0; i < numObjects; i++)
    {
        const Size objectSize = sizes.random() * sizeof(u32);
        Allocator::Range args = { 0, objectSize, 0 };

        testAssert(pa.allocate(args) == Allocator::Success);
        testAssert(args.address != ZERO);
        testAssert(args.size == objectSize);

        objects.insert((u8 *)(args.address));
    }

    // Generate index numbers in randomized order
    indexes.unique(numObjects);

    // Release all objects in randomized order
    for (Size i = 0; i < objects.count(); i++)
    {
        testAssert(pa.release((Address)(objects[indexes[i]])) == Allocator::Success);
    }

    // All pools must be released now
    testAssert(pa.size() == 0);
    testAssert(pa.available() == 0);

    return OK;
}

TestCase(PoolIndex)
{
    DummyParent parent;
    PoolAllocator pa(&parent);
    Allocator::Range args = { 0, 0, 0 };

    // Object size zero does not fit anywhere
    testAssert(pa.allocate(args) == Allocator::InvalidSize);

    // Verify that new objects are placed in the correct Pool index
    for (Size i = 2; i < 22; i++)
    {
        const Size objectSize = 1U << i;
        const Size actualSize = sizeof(PoolAllocator::ObjectPrefix) +
                                sizeof(PoolAllocator::ObjectPostfix) +
                                objectSize;
        Size index = 2;

        // Calculate the expected index for the Pool
        for (; index < 22; index++) {
            if ((1U << index) >= actualSize)
                break;
        }

        // Perform allocation
        args.size = objectSize;
        testAssert(pa.allocate(args) == Allocator::Success);

        // Verify the correct Pool is used
        const PoolAllocator::ObjectPrefix *prefix =
            (const PoolAllocator::ObjectPrefix *) (args.address - sizeof(PoolAllocator::ObjectPrefix));

        testAssert(pa.m_pools[index] != ZERO);
        testAssert(prefix->pool->index == index);
        testAssert(prefix->pool == pa.m_pools[index]);
        testAssert(pa.release(args.address) == Allocator::Success);
    }

    return OK;
}

TestCase(PoolLayout)
{
    DummyParent parent;
    PoolAllocator pa(&parent);
    Allocator::Range args = { 0, 64, 0 };
    Vector<u8 *> objects;

    // Check that a Pool has a correct in-memory layout
    testAssert(pa.allocate(args) == Allocator::Success);
    testAssert(pa.m_pools[7] != ZERO);
    testAssert(pa.m_pools[7]->m_array.m_array == ((u8 *)(pa.m_pools[7])) + sizeof(PoolAllocator::Pool));
    testAssert(args.address >= (Address)(pa.m_pools[7]) + sizeof(PoolAllocator::Pool) + pa.m_pools[7]->bitmapSize);

    // Release back object
    testAssert(pa.release(args.address) == Allocator::Success);
    return OK;
}

TestCase(PoolPrevNext)
{
    DummyParent parent;
    PoolAllocator pa(&parent);
    Allocator::Range args = { 0, 64, 0 };
    Vector<u8 *> objects;

    // Fillup the first Pool
    for (Size i = 0; i < 128; i++)
    {
        // Perform the allocation
        testAssert(pa.allocate(args) == Allocator::Success);
        testAssert(args.alignment == 0);
        testAssert(args.size == 64);
        testAssert(pa.m_pools[7] != ZERO);
        testAssert(pa.m_pools[7]->prev == ZERO);
        testAssert(pa.m_pools[7]->next == ZERO);

        objects.insert((u8 *)(args.address));
    }
    PoolAllocator::Pool *currentPool = pa.m_pools[7];

    // Allocate once more. This should create a second Pool
    testAssert(pa.allocate(args) == Allocator::Success);
    testAssert(pa.m_pools[7] != ZERO);
    testAssert(pa.m_pools[7]->prev == ZERO);
    testAssert(pa.m_pools[7]->next != ZERO);
    testAssert(pa.m_pools[7]->next->prev == pa.m_pools[7]);
    testAssert(pa.m_pools[7]->next->next == ZERO);

    // Release back the object. This should release the second Pool also.
    testAssert(pa.release(args.address) == Allocator::Success);
    testAssert(pa.m_pools[7] != ZERO);
    testAssert(pa.m_pools[7] == currentPool);
    testAssert(pa.m_pools[7]->prev == ZERO);
    testAssert(pa.m_pools[7]->next == ZERO);

    // Release back all objects
    for (Size i = 0; i < objects.count(); i++)
    {
        testAssert(pa.release((Address) objects[i]) == Allocator::Success);
    }

    // Last Pool is also released
    testAssert(pa.m_pools[7] == ZERO);

    return OK;
}

TestCase(PoolObjectSize)
{
    DummyParent parent;
    PoolAllocator pa(&parent);

    // Each Pool gets a power-of-two object size
    for (Size index = 2; index <= 27; index++)
    {
        testAssert(pa.calculateObjectSize(index) == (1U << index));
    }

    return OK;
}

TestCase(PoolObjectCount)
{
    DummyParent parent;
    PoolAllocator pa(&parent);

    // Verify pools get the desired number of objects
    testAssert(pa.calculateObjectCount(4) == 4 * 1024);
    testAssert(pa.calculateObjectCount(32) == 512);
    testAssert(pa.calculateObjectCount(2048) == 8);
    testAssert(pa.calculateObjectCount(4096) == 4);
    testAssert(pa.calculateObjectCount(16 * 1024) == 1);
    testAssert(pa.calculateObjectCount(1024 * 1024) == 1);

    return OK;
}
