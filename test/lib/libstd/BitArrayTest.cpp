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
#include <TestData.h>
#include <TestMain.h>
#include <BitArray.h>

TestCase(BitArrayConstruct)
{
    TestData<Size> sizes;
    BitArray ba(sizes.uvalue(128, 64));

    testAssert(ba.m_array != NULL);
    testAssert(ba.m_size == sizes[0]);
    testAssert(ba.m_set  == 0);
    testAssert(ba.count(false) == sizes[0]);

    for (Size i = 0; i < sizes[0]; i++)
        testAssert(!ba[i]);

    return OK;
}

TestCase(BitArraySet)
{
    TestData<Size> sizes;
    BitArray ba(sizes.uvalue(128, 64));

    ba.set(1, true);
    testAssert(ba.isSet(1));
    testAssert(!ba.isSet(0));

    return OK;
}

TestCase (BitArraySkip)
{
    return SKIP;
}
