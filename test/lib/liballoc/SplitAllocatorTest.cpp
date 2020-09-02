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
#include <SplitAllocator.h>

TestCase(SplitConstruct)
{
    TestInt<uint> physAddresses((UINT_MAX/2) + 1, UINT_MAX);
    TestInt<uint> virtAddresses(UINT_MAX/4, UINT_MAX/2);
    const Address physBase = physAddresses.random() & PAGEMASK;
    const Address virtBase = virtAddresses.random() & PAGEMASK;
    const Size allocSize = 16 * PAGESIZE;

    const Allocator::Range physRange = { physBase, allocSize, PAGESIZE };
    const Allocator::Range virtRange = { virtBase, allocSize, PAGESIZE };
    SplitAllocator sa(physRange, virtRange, PAGESIZE);

    // Verify initial state of the object
    testAssert(sa.available() == allocSize);
    testAssert(sa.base() == physBase);
    testAssert(sa.size() == allocSize);
    testAssert(sa.alignment() == PAGESIZE);
    testAssert(sa.m_pageSize == PAGESIZE);
    testAssert(sa.m_virtRange.address == virtBase);

    return OK;
}

TestCase(SplitAllocatePhysical)
{
    TestInt<uint> physAddresses((UINT_MAX/2) + 1, UINT_MAX);
    TestInt<uint> virtAddresses(UINT_MAX/4, UINT_MAX/2);
    const Address physBase = physAddresses.random() & PAGEMASK;
    const Address virtBase = virtAddresses.random() & PAGEMASK;
    const Size allocSize = 16 * PAGESIZE;

    const Allocator::Range physRange = { physBase, allocSize, PAGESIZE };
    const Allocator::Range virtRange = { virtBase, allocSize, PAGESIZE };
    SplitAllocator sa(physRange, virtRange, PAGESIZE);

    // Allocate ten physical pages
    for (Size i = 0; i < 10; i++)
    {
        Allocator::Range args = { 0, PAGESIZE, 0 };

        testAssert(sa.allocate(args) == Allocator::Success);
        testAssert(args.address == physBase + (i * PAGESIZE));
        testAssert(args.size == PAGESIZE);
    }

    testAssert(sa.available() == 6 * PAGESIZE);
    return OK;
}

TestCase(SplitAllocatePhysicalVirtual)
{
    TestInt<uint> physAddresses((UINT_MAX/2) + 1, UINT_MAX);
    TestInt<uint> virtAddresses(UINT_MAX/4, UINT_MAX/2);
    const Address physBase = physAddresses.random() & PAGEMASK;
    const Address virtBase = virtAddresses.random() & PAGEMASK;
    const Size allocSize = 16 * PAGESIZE;

    const Allocator::Range physRange = { physBase, allocSize, PAGESIZE };
    const Allocator::Range virtRange = { virtBase, allocSize, PAGESIZE };
    SplitAllocator sa(physRange, virtRange, PAGESIZE);

    // Allocate ten physical pages with virtual address translation
    for (Size i = 0; i < 10; i++)
    {
        Allocator::Range phys = { 0, PAGESIZE, 0 };
        Allocator::Range virt = { 0, PAGESIZE, 0 };

        testAssert(sa.allocate(phys, virt) == Allocator::Success);
        testAssert(phys.address == physBase + (i * PAGESIZE));
        testAssert(phys.size == PAGESIZE);
        testAssert(virt.address == virtBase + (i * PAGESIZE));
        testAssert(virt.size == PAGESIZE);
        testAssert(virt.address == sa.toVirtual(phys.address));
        testAssert(phys.address == sa.toPhysical(virt.address));
    }

    testAssert(sa.available() == 6 * PAGESIZE);
    return OK;
}

TestCase(SplitAllocateFull)
{
    TestInt<uint> physAddresses((UINT_MAX/2) + 1, UINT_MAX);
    TestInt<uint> virtAddresses(UINT_MAX/4, UINT_MAX/2);
    const Address physBase = physAddresses.random() & PAGEMASK;
    const Address virtBase = virtAddresses.random() & PAGEMASK;
    const Size allocSize = 10 * PAGESIZE;

    const Allocator::Range physRange = { physBase, allocSize, PAGESIZE };
    const Allocator::Range virtRange = { virtBase, allocSize, PAGESIZE };
    SplitAllocator sa(physRange, virtRange, PAGESIZE);
    Allocator::Range args = { 0, PAGESIZE, 0 };

    // Allocate ten pages
    for (Size i = 0; i < 10; i++)
    {
        testAssert(sa.allocate(args) == Allocator::Success);
        testAssert(args.address == physBase + (i * PAGESIZE));
        testAssert(args.size == PAGESIZE);
    }

    // Now we are full. Allocation should fail
    testAssert(sa.allocate(args) == Allocator::OutOfMemory);
    testAssert(sa.available() == 0);

    return OK;
}

TestCase(SplitAllocateRelease)
{
    TestInt<uint> physAddresses((UINT_MAX/2) + 1, UINT_MAX);
    TestInt<uint> virtAddresses(UINT_MAX/4, UINT_MAX/2);
    const Address physBase = physAddresses.random() & PAGEMASK;
    const Address virtBase = virtAddresses.random() & PAGEMASK;
    const Size allocSize = 10 * PAGESIZE;

    const Allocator::Range physRange = { physBase, allocSize, PAGESIZE };
    const Allocator::Range virtRange = { virtBase, allocSize, PAGESIZE };
    SplitAllocator sa(physRange, virtRange, PAGESIZE);
    Allocator::Range args = { 0, PAGESIZE, 0 };

    // Allocate ten pages
    for (Size i = 0; i < 10; i++)
    {
        testAssert(sa.allocate(args) == Allocator::Success);
        testAssert(args.address == physBase + (i * PAGESIZE));
        testAssert(args.size == PAGESIZE);
    }

    // Now we are full. Release back all pages one-by-one
    testAssert(sa.available() == 0);
    for (Size i = 0; i < 10; i++)
    {
        testAssert(sa.release(physBase + (i * PAGESIZE)) == Allocator::Success);
    }

    // All pages are free again. Confirm re-allocation works
    testAssert(sa.available() == allocSize);
    testAssert(sa.allocate(args) == Allocator::Success);
    testAssert(args.size == PAGESIZE);

    // With the BitAllocator, the search for the allocation address
    // begins at the last succesfully allocated address. In this case,
    // that is the last page.
    testAssert(args.address == physBase + allocSize - PAGESIZE);
    return OK;
}
