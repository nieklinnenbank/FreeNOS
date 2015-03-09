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

#include <TestCase.h>
#include <TestRunner.h>
#include <BubbleAllocator.h>

TestCase(BubbleConstruct)
{
#warning Use random test data. Use a TestData class for this.

    BubbleAllocator *ba = new BubbleAllocator(0x10000, 4096);

    testAssert(ba->start == (u8 *) 0x10000);
    testAssert(ba->size == 4096);

    delete ba;
    return OK;
}

TestCase(BubbleAlloc)
{
    BubbleAllocator *ba = new BubbleAllocator(0x10000, 4096);
    Size sz = 1024, big = 8192;

    testAssert(ba->allocate(&sz) == 0x10000);
    testAssert(ba->current == (u8 *) 0x10000 + sz);
    testAssert(ba->allocate(&big) == 0);

    delete ba;
    return OK;
}

int main(int argc, char **argv)
{
    TestRunner tests(argc, argv);
    return tests.run();
}
