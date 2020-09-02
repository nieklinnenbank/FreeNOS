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
#include <BitAllocator.h>

TestCase(BitConstruct)
{
    TestInt<uint> addresses(UINT_MIN, UINT_MAX);

    const Size chunkSize = 32;
    const Size rangeSize = chunkSize * 64;
    const Address allocBase = addresses.random() & PAGEMASK;
    const Allocator::Range range = { allocBase, rangeSize, sizeof(u32) };

    BitAllocator ba(range, chunkSize);

    // Verify initial state of the object
    testAssert(ba.m_chunkSize == chunkSize);
    testAssert(ba.available() == rangeSize);
    testAssert(ba.base() == allocBase);
    testAssert(ba.size() == rangeSize);
    testAssert(ba.alignment() == sizeof(u32));
    testAssert(!ba.isAllocated(allocBase));

    return OK;
}

TestCase(BitAllocateChunks)
{
    TestInt<uint> addresses(UINT_MIN, UINT_MAX);

    const Size chunkSize = 32;
    const Size rangeSize = chunkSize * 64;
    const Address allocBase = addresses.random() & PAGEMASK;
    const Allocator::Range range = { allocBase, rangeSize, sizeof(u32) };

    BitAllocator ba(range, chunkSize);

    // Allocate ten chunks
    for (Size i = 0; i < 10; i++)
    {
        Allocator::Range args = { 0, chunkSize, 0 };

        testAssert(ba.allocate(args) == Allocator::Success);
        testAssert(args.address == allocBase + (chunkSize * i));
        testAssert(args.size == chunkSize);
        testAssert(ba.isAllocated(allocBase + (chunkSize * i)));
    }

    // The ten chunks are allocated, the rest is free
    for (Size i = 0; i < rangeSize; i += chunkSize)
    {
        if (i < (chunkSize * 10))
        {
            testAssert(ba.isAllocated(allocBase + i));
        }
        else
        {
            testAssert(!ba.isAllocated(allocBase + i));
        }
    }

    return OK;
}

TestCase(BitAllocateBytes)
{
    TestInt<uint> addresses(UINT_MIN, UINT_MAX);

    const Size chunkSize = 32;
    const Size rangeSize = chunkSize * 256;
    const Address allocBase = addresses.random() & PAGEMASK;
    const Allocator::Range range = { allocBase, rangeSize, sizeof(u32) };

    BitAllocator ba(range, chunkSize);

    // Make allocations of one byte up to the chunkSize.
    // For each allocation, a full chunk will be used.
    for (Size i = 1; i <= chunkSize; i++)
    {
        Allocator::Range args = { 0, i, 0 };

        testAssert(ba.allocate(args) == Allocator::Success);
        testAssert(args.address == allocBase + (chunkSize * (i-1)));
        testAssert(args.size == i);
        testAssert(ba.isAllocated(allocBase + (chunkSize * (i-1))));
    }

    // We should now have 32 chunks allocated, the rest is free
    for (Size i = 0; i < rangeSize; i += chunkSize)
    {
        if (i < (chunkSize * chunkSize))
        {
            testAssert(ba.isAllocated(allocBase + i));
        }
        else
        {
            testAssert(!ba.isAllocated(allocBase + i));
        }
    }

    // Repeat allocating with bytes above the chunkSize.
    // Now for each allocation, more than one chunk needs to be used.
    Address allocAddr = allocBase + (chunkSize * chunkSize);

    for (Size i = chunkSize + 1; i <= (chunkSize * 3); i++)
    {
        Allocator::Range args = { 0, i, 0 };

        testAssert(ba.allocate(args) == Allocator::Success);
        testAssert(args.address == allocAddr);
        testAssert(args.size == i);
        testAssert(ba.isAllocated(allocAddr));

        // Increment allocation address. If the size isn't a multiple
        // of the chunkSize, the size is rounded up to the nearest multiple.
        allocAddr += (chunkSize * (i / chunkSize));
        if (i % chunkSize)
            allocAddr += chunkSize;
    }

    // Everything up to allocAddress should be allocated, the rest is free.
    for (Address i = allocBase; i < allocBase + rangeSize; i += chunkSize)
    {
        if (i < allocAddr)
        {
            testAssert(ba.isAllocated(i));
        }
        else
        {
            testAssert(!ba.isAllocated(i));
        }
    }

    return OK;
}

TestCase(BitAllocateAlignment)
{
    const Size chunkSize = 32;
    const Size rangeSize = chunkSize * 64;
    const Allocator::Range range = { chunkSize * 999, rangeSize, sizeof(u32) };

    BitAllocator ba(range, chunkSize);
    Allocator::Range args = { 0, chunkSize, 0 };

    // Allocate a chunk with the default alignment (which is one chunk)
    testAssert(ba.allocate(args) == Allocator::Success);
    testAssert((args.address % chunkSize) == 0);
    testAssert(args.size == chunkSize);

    // Make ten allocations with the default alignment
    for (Size i = 1; i <= 10; i++)
    {
        args.size = i;
        testAssert(ba.allocate(args) == Allocator::Success);
        testAssert((args.address % chunkSize) == 0);
    }

    // Make another ten allocations with alignment of three chunks
    for (Size i = 1; i <= 10; i++)
    {
        args.size = i;
        args.alignment = chunkSize * 3;
        testAssert(ba.allocate(args) == Allocator::Success);
        testAssert((args.address % (chunkSize * 3)) == 0);
    }

    // Now verify that alignments which are not the multiple of
    // a chunk size will be rejected
    args.size = 1;
    args.alignment = chunkSize + 1;
    testAssert(ba.allocate(args) == Allocator::InvalidAlignment);

    return OK;
}

TestCase(BitAllocateStartFrom)
{
    TestInt<uint> addresses(UINT_MIN, UINT_MAX);

    const Size chunkSize = 32;
    const Size rangeSize = chunkSize * 64;
    const Address allocBase = addresses.random() & PAGEMASK;
    const Allocator::Range range = { allocBase, rangeSize, sizeof(u32) };

    BitAllocator ba(range, chunkSize);
    Allocator::Range args = { 0, chunkSize, 0 };

    // Allocate one chunk, start searching at the 2nd bit
    testAssert(ba.allocateFrom(args, 1) == Allocator::Success);
    testAssert(args.address == allocBase + chunkSize);
    testAssert(args.size == chunkSize);
    testAssert(ba.isAllocated(allocBase + chunkSize));

    // Allocate one chunk, start searching at the 3rd bit
    testAssert(ba.allocateFrom(args, 2) == Allocator::Success);
    testAssert(args.address == allocBase + (chunkSize * 2));
    testAssert(args.size == chunkSize);
    testAssert(ba.isAllocated(allocBase + (chunkSize * 2)));

    return OK;
}

TestCase(BitAllocateSpecific)
{
    TestInt<uint> addresses(UINT_MIN, UINT_MAX);

    const Size chunkSize = 32;
    const Size rangeSize = chunkSize * 256;
    const Address allocBase = addresses.random() & PAGEMASK;
    const Allocator::Range range = { allocBase, rangeSize, sizeof(u32) };

    BitAllocator ba(range, chunkSize);

    // Make allocations with explicit addresses
    for (Size i = 1; i <= chunkSize; i++)
    {
        testAssert(ba.allocateAt(allocBase + (chunkSize * (i-1))) == Allocator::Success);
        testAssert(ba.isAllocated(allocBase + (chunkSize * (i-1))));
    }

    // We should now have 32 chunks allocated, the rest is free
    for (Size i = 0; i < rangeSize; i += chunkSize)
    {
        if (i < (chunkSize * chunkSize))
        {
            testAssert(ba.isAllocated(allocBase + i));
        }
        else
        {
            testAssert(!ba.isAllocated(allocBase + i));
        }
    }

    return OK;
}

TestCase(BitAllocateFull)
{
    TestInt<uint> addresses(UINT_MIN, UINT_MAX);
    const Size chunkSize = PAGESIZE;
    const Size rangeSize = chunkSize * 10;
    const Allocator::Range range = { 0xabcd0000, rangeSize, sizeof(u32) };

    BitAllocator ba(range, chunkSize);
    Allocator::Range args = { 0, PAGESIZE, 0 };

    // Allocate ten pages
    for (Size i = 0; i < 10; i++)
    {
        testAssert(ba.allocate(args) == Allocator::Success);
        testAssert(args.address == range.address + (PAGESIZE * i));
        testAssert(args.size == PAGESIZE);
    }

    // Now we are full. Allocation should fail
    testAssert(ba.allocate(args) == Allocator::OutOfMemory);
    testAssert(ba.available() == 0);

    return OK;
}

TestCase(BitRelease)
{
    TestInt<uint> addresses(UINT_MIN, UINT_MAX);
    const Size chunkSize = PAGESIZE;
    const Size rangeSize = chunkSize * 10;
    const Allocator::Range range = { 0xabcd0000, rangeSize, sizeof(u32) };

    BitAllocator ba(range, chunkSize);
    Allocator::Range args = { 0, PAGESIZE, 0 };

    // Allocate ten pages
    for (Size i = 0; i < 10; i++)
    {
        testAssert(ba.allocate(args) == Allocator::Success);
        testAssert(args.address == range.address + (PAGESIZE * i));
        testAssert(args.size == PAGESIZE);
    }

    // Now we are full. Release back all chunks one-by-one
    for (Size i = 0; i < 10; i++)
    {
        testAssert(ba.release(range.address + (i * chunkSize)) == Allocator::Success);
        testAssert(!ba.isAllocated(range.address + (i * chunkSize)));

        // Verify that the rest is still allocated
        for (Size j = i + 1; j < 10; j++)
        {
            testAssert(ba.isAllocated(range.address + (j * chunkSize)));
        }
    }

    // All chunks are free again. Confirm re-allocation works
    testAssert(ba.available() == rangeSize);
    testAssert(ba.allocate(args) == Allocator::Success);
    testAssert(args.size == PAGESIZE);

    // Note that the search for the allocation address begins at the
    // last successfully allocated address, which here is the last chunk
    testAssert(args.address == range.address + rangeSize - chunkSize);
    return OK;
}
