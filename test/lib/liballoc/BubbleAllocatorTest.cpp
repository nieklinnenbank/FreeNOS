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

#include <FreeNOS/System/Constant.h>
#include <TestCase.h>
#include <TestRunner.h>
#include <TestData.h>
#include <TestMain.h>
#include <BubbleAllocator.h>

TestCase(BubbleConstruct)
{
    TestData<uint> data;
    Address addr = data.uvalue();
    Size size = data.uvalue(16, 1) * PAGESIZE;

    BubbleAllocator *ba = new BubbleAllocator(addr, size);

    testAssert(ba->start == (u8 *) addr);
    testAssert(ba->size == size);

    delete ba;
    return OK;
}

TestCase(BubbleAlloc)
{
    TestData<uint> data;
    Address addr = data.uvalue();
    Size size = data.uvalue(16, 1) * PAGESIZE;

    BubbleAllocator *ba = new BubbleAllocator(addr, size);
    Size sz = 1024, big = size * 2;

    testAssert(ba->allocate(&sz) == addr);
    testAssert(ba->current == (u8 *) addr + sz);
    testAssert(ba->allocate(&big) == 0);

    delete ba;
    return OK;
}
