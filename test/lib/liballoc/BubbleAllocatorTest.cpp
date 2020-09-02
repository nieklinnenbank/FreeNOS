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

#include <FreeNOS/Constant.h>
#include <TestCase.h>
#include <TestRunner.h>
#include <TestInt.h>
#include <TestMain.h>
#include <BubbleAllocator.h>

TestCase(BubbleConstruct)
{
    const Size rangeSize = PAGESIZE * 64;
    TestInt<uint> addresses(UINT_MIN, UINT_MAX - rangeSize);
    const Allocator::Range range = { addresses.random(), rangeSize, sizeof(u32) };
    BubbleAllocator ba(range);

    // Verify initial state of the object
    testAssert(ba.m_allocated == 0);
    testAssert(ba.base() == addresses[0]);
    testAssert(ba.size() == rangeSize);
    testAssert(ba.alignment() == sizeof(u32));

    return OK;
}

TestCase(BubbleAllocate)
{
    const Size rangeSize = PAGESIZE * 64;
    TestInt<uint> addresses(UINT_MIN, UINT_MAX - rangeSize);
    const Allocator::Range range = { addresses.random(), rangeSize, sizeof(u32) };
    BubbleAllocator ba(range);
    Allocator::Range args = { 0, PAGESIZE, sizeof(u32) };

    // Allocate ten pages
    for (Size i = 0; i < 10; i++)
    {
        testAssert(ba.allocate(args) == Allocator::Success);
        testAssert(args.address == addresses[0] + (PAGESIZE * i));
        testAssert(args.size == PAGESIZE);
        testAssert(ba.m_allocated == PAGESIZE * (i + 1));
    }

    // Allocate just 8 bytes
    args.size = 8;
    testAssert(ba.allocate(args) == Allocator::Success);
    testAssert(args.address == addresses[0] + (PAGESIZE * 10));
    testAssert(args.size == 8);
    testAssert(ba.m_allocated == (PAGESIZE * 10) + 8);

    // Allocate another page
    args.size = PAGESIZE;
    args.alignment = 8192;
    testAssert(ba.allocate(args) == Allocator::Success);
    testAssert(args.address == addresses[0] + (PAGESIZE * 10) + (8));
    testAssert(args.size == PAGESIZE);
    testAssert(ba.m_allocated == (PAGESIZE * 11) + (8));

    return OK;
}

TestCase(BubbleAlignment)
{
    const Size rangeSize = PAGESIZE * 64;
    const Allocator::Range range = { 0xabcd0000, rangeSize, PAGESIZE };
    BubbleAllocator ba(range);
    Allocator::Range args = { 0, PAGESIZE, sizeof(u32) };

    // Allocate one page
    testAssert(ba.allocate(args) == Allocator::Success);
    testAssert(args.address == range.address);
    testAssert((args.address % PAGESIZE) == 0);
    testAssert(args.size == PAGESIZE);
    testAssert(ba.m_allocated == PAGESIZE);

    // Try to allocate 16 bytes
    args.size = 16;
    testAssert(ba.allocate(args) == Allocator::Success);
    testAssert(args.address == range.address + PAGESIZE);
    testAssert((args.address % PAGESIZE) == 0);
    testAssert(args.size == 16);
    testAssert(ba.m_allocated == PAGESIZE * 2);

    // Allocate another page, should be PAGESIZE aligned
    args.size = PAGESIZE;
    testAssert(ba.allocate(args) == Allocator::Success);
    testAssert(args.address == range.address + (PAGESIZE * 2));
    testAssert((args.address % PAGESIZE) == 0);
    testAssert(args.size == PAGESIZE);
    testAssert(ba.m_allocated == PAGESIZE * 3);

    return OK;
}

TestCase(BubbleFull)
{
    TestInt<uint> addresses(UINT_MIN, UINT_MAX);
    const Size rangeSize = PAGESIZE * 10;
    const Allocator::Range range = { 0xabcd0000, rangeSize, sizeof(u32) };
    BubbleAllocator ba(range);
    Allocator::Range args = { 0, PAGESIZE, sizeof(u32) };

    // Allocate ten pages
    for (Size i = 0; i < 10; i++)
    {
        testAssert(ba.allocate(args) == Allocator::Success);
        testAssert(args.address == range.address + (PAGESIZE * i));
        testAssert(args.size == PAGESIZE);
        testAssert(ba.m_allocated == PAGESIZE * (i + 1));
    }

    // Now we are full. Allocation should fail
    testAssert(ba.allocate(args) == Allocator::OutOfMemory);
    testAssert(ba.m_allocated == PAGESIZE * 10);

    return OK;
}

TestCase(BubbleRelease)
{
    const Size rangeSize = PAGESIZE * 64;
    TestInt<uint> addresses(UINT_MIN, UINT_MAX - rangeSize);
    const Allocator::Range range = { addresses.random(), rangeSize, sizeof(u32) };
    BubbleAllocator ba(range);
    Allocator::Range args = { 0, PAGESIZE, sizeof(u32) };

    // Allocate ten pages
    for (Size i = 0; i < 10; i++)
    {
        testAssert(ba.allocate(args) == Allocator::Success);
        testAssert(args.address == addresses[0] + (PAGESIZE * i));
        testAssert(args.size == PAGESIZE);
        testAssert(ba.m_allocated == PAGESIZE * (i + 1));
    }

    // Releasing should not work
    testAssert(ba.release(addresses[0]) != Allocator::Success);
    testAssert(ba.m_allocated == PAGESIZE * 10);

    return OK;
}
