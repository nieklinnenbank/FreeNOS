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
    TestData<uint> uints;
    TestData<int> ints;
    Array<int, 64> a;

    // Check the array has the correct size
    testAssert(a.size() == 64);
    return OK;
}

TestCase(ArrayFill)
{
    TestData<uint> uints;
    TestData<int> ints;
    Array<int, 64> a;

    // Fill the array with a random value
    a.fill(ints.value());

    // Check the array is completely filled with the value
    for (Size i = 0; i < 64; i++)
    {
        testAssert(a[i] == ints[0]);
    }
    return OK;
}

TestCase(ArrayOverflow)
{
    TestData<uint> uints;
    Array<int, 64> a;

    // Try to overflow the array
    testAssert(!a.put(64, 0));
    testAssert(!a.put(65, 0));

    return OK;
}

TestCase(ArrayOverwrite)
{
    TestData<uint> uints;
    TestData<int> ints;
    Array<int, 64> a;

    // Write first item
    a.put(0, ints.value());
    testAssert(a[0] == ints[0]);

    // Overwrite first item many times
    for (Size i = 0; i < 64; i++)
    {
        a.put(0, ints.value());
        testAssert(a[0] == ints[i+1]);
    }
    return OK;
}

TestCase(ArrayPutSeq)
{
    TestData<uint> udata;
    TestData<int> idata;
    Array<int, 256> a;

    // Clear the array
    a.fill(0);

    // Fill the array sequentially
    for (Size i = 0; i < 256; i++)
    {
        // Generate value and insert it at the current index
        a[i] = idata.value();

        // Assert that all previously inserted values are still there
        // and all other places are ZERO.
        for (Size j = 0; j < 256; j++)
        {
            if (j <= i)
            {
                testAssert(a[j] == idata[j]);
            }
            else
            {
                testAssert(a[j] == ZERO);
            }
        }
    }
    return OK;
}

TestCase(ArrayPutRandom)
{
    TestData<uint> sizes;
    TestData<int> ints;
    uint count = sizes.uvalue(256, 64);
    Array<int, 256> a;

    // Clear the array
    a.fill(0);

    // TODO: put this in libtest? to generate a random array of ints?

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
        a.put(indexes[i], ints.value());

        // Assert that all previously inserted values are still there
        // and that empty places are ZERO.
        for (Size j = 0; j < count; j++)
        {
            if (j <= i)
            {
                testAssert(a[indexes[j]] == ints[j]);
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

TestCase(ArrayCompare)
{
    TestData<uint> udata;
    TestData<int> idata;
    Array<int, 64> a1, a2;

    // Fill the arrays completely
    for (Size i = 0; i < 64; i++)
    {
        // Generate value and insert it at the current index
        a1.put(i, idata.value());
        a2.put(i, idata[i]);
    }

    // Both arrays must be equal
    testAssert(a1.compareTo(a2) == 0);
    testAssert(a1.equals(a2));
    testAssert(a1.size() == a2.size());

    // Change one item. Arrays cannot be equal
    a1.put(0, ~(a1[0]));
    testAssert(a1.compareTo(a2) != 0);
    testAssert(!a1.equals(a2));
    testAssert(a1.size() == a2.size());
    return OK;
}
