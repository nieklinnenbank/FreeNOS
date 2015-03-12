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
#include <Array.h>

TestCase(ArrayConstruct)
{
    TestData<uint> data;
    uint size = data.uvalue(64, 16);
    Array<int> a(size);

    // Check the array is empty with the correct size
    testAssert(a.size() == data.get(0));
    testAssert(a.count() == 0);
    testAssert(a.findEmpty() == 0);

    for (Size i = 0; i < a.m_size; i++)
    {
        testAssert(a.m_array[i] == ZERO);
    }
    return OK;
}

TestCase(ArrayOverflow)
{
    TestData<uint> udata;
    TestData<int> idata;
    uint size = udata.uvalue(64, 16);
    Array<int> a(size);

    // First completely fill the array.
    for (Size i = 0; i < a.m_size; i++)
    {
        a.insert(idata.value());
        testAssert(a.get(i) == idata.get(i));
    }
    // Overflow the array by one.
    testAssert(a.insert(idata.value()) == -1);
    testAssert(!a.insert(a.m_size + 1, idata.value()));
    testAssert(a.size() == a.count());
    return OK;
}

TestCase(ArrayInsertSeq)
{
    TestData<uint> udata;
    TestData<int> idata;
    uint size  = udata.value(256, 128);
    Array<int> a(size);

    // Fill the array sequentially
    for (Size i = 0; i < size; i++)
    {
        // Generate value and insert it at the current index
        a.insert(i, idata.value());

        // Assert that all previously inserted values are still there
        // and all other places are ZERO.
        for (Size j = 0; j < size; j++)
        {
            if (j <= i)
            {
                testAssert(a[j] == idata.get(j));
            }
            else
            {
                testAssert(a[j] == ZERO);
            }
        }
    }
    return OK;
}

TestCase(ArrayInsertRandom)
{
    TestData<uint> sizes;
    TestData<int> ints;
    uint size  = sizes.uvalue(256, 128);
    uint count = sizes.uvalue(size, 64);
    Array<int> a(size);

    // Allocate buffer to store random indexes
    Size *indexes = new Size[count];
    for (Size i = 0; i < count; i++)
        indexes[i] = i;

    // Randomize the indexes buffer by swapping
    for (Size i = 0; i < count; i++)
    {
        Size tmp = indexes[i];
        Size idx = sizes.uvalue(count - 1, 0);
        indexes[i] = indexes[idx];
        indexes[idx] = tmp;
    }

    // Fill the array randomly
    for (Size i = 0; i < count; i++)
    {    
        // Generate value and insert it at a random index
        a.insert(indexes[i], ints.value());

        // Assert that all previously inserted values are still there
        // and that empty places are ZERO.
        for (Size j = 0; j < count; j++)
        {
            if (j <= i)
            {
                testAssert(a[indexes[j]] == ints.get(j));
            }
            else
            {
                testAssert(a[indexes[j]] == ZERO);
            }
        }
    }
    delete[] indexes;
    return OK;
}

TestCase(ArrayRemove)
{
    return SKIP;
}

TestCase(ArrayClear)
{
    return SKIP;
}

TestCase(ArrayClone)
{
    return SKIP;
}

TestCase(ArrayCompare)
{
    return SKIP;
}
