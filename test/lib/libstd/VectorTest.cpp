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
#include <Vector.h>

TestCase(VectorConstruct)
{
    TestData<uint> data;
    uint size = data.uvalue(64, 16);
    Vector<int> a(size);

    // Check the vector is empty with the correct size
    testAssert(a.size() == data[0]);
    testAssert(a.count() == 0);
    return OK;
}

TestCase(VectorFill)
{
    TestData<uint> data;
    TestData<int> ints;
    uint size = data.uvalue(64, 16);
    Vector<int> a(size);

    // Fill the vector with a random value
    a.fill(ints.value());

    // Check the vector is completely filled with the value
    for (Size i = 0; i < a.count(); i++)
    {
        testAssert(a[i] == ints[0]);
    }
    // Administration should be correct
    testAssert(a.size() == size);
    testAssert(a.count() == size);
    return OK;
}

TestCase(VectorResize)
{
    TestData<uint> udata;
    TestData<int> idata;
    uint size = udata.uvalue(64, 16);
    Vector<int> a(size);

    // First completely fill the vector three times its original size.
    for (Size i = 0; i < (size * 3); i++)
    {
        testAssert(a.put(idata.value()) == i);
        testAssert(a.at(i) == idata[i]);
    }
    // Check administration counters
    testAssert(a.count() == size * 3);
    testAssert(a.size() >= size * 3);
    return OK;
}

TestCase(VectorPutMultiple)
{
    TestData<uint> udata;
    TestData<int> idata;
    uint size  = udata.uvalue(256, 128);
    Vector<int> a(size);

    // Insert one value.
    a.put(0, idata.value());
    testAssert(a[0] == idata[0]);

    // Insert another value on the same index.
    // It must overwrite but keep administration correct.
    a.put(0, idata.value());
    testAssert(a[0] == idata[1]);
    testAssert(a.count() == 1);
    testAssert(a.size() == size);
    return OK;
}

TestCase(VectorPutSeq)
{
    TestData<uint> udata;
    TestData<int> idata;
    uint size  = udata.uvalue(256, 128);
    Vector<int> a(size);

    // Clear the vector
    a.fill(0);

    // Fill the vector sequentially
    for (Size i = 0; i < size; i++)
    {
        // Generate value and insert it at the current index
        a.put(i, idata.value());

        // Assert that all previously inserted values are still there
        // and all other places are ZERO.
        for (Size j = 0; j < size; j++)
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

TestCase(VectorPutRandom)
{
    TestData<uint> sizes;
    TestData<int> ints;
    uint size  = sizes.uvalue(256, 128);
    uint count = sizes.uvalue(size, 64);
    Vector<int> a(size);

    // Clear the vector
    a.fill(0);

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

    // Fill the vector randomly
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

TestCase(VectorRemoveOne)
{
    TestData<uint> sizes;
    TestData<int> ints;
    uint size  = sizes.uvalue(256, 128);
    Vector<int> a(size);

    // Completely fill the vector sequentially
    for (Size i = 0; i < size; i++)
        // Generate value and insert it at the current index
        a.put(i, ints.value());

    // Remove one item randomly from the vector
    Size idx = sizes.uvalue(size, 0);
    testAssert(a.remove(idx));
    testAssert(a.count() == size - 1);
    testAssert(a.size() == size);

    // Removing non-existing item should fail.
    testAssert(!a.remove(size));

    // Check that all items before the item are still inside, except the item removed.
    for (Size i = 0; i < idx; i++)
    {
        testAssert(a[i] == ints[i]);
    }
    // After the removed item.
    for (Size i = idx; i < size - 1; i++)
    {
        testAssert(a[i] == ints[i+1]);
    }
    return OK;
}

TestCase(VectorClear)
{
    TestData<uint> udata;
    TestData<int> idata;
    uint size  = udata.uvalue(256, 128);
    Vector<int> a(size);

    // Fill the vector completely
    for (Size i = 0; i < size; i++)
    {
        // Generate value and insert it at the current index
        a.put(i, idata.value());
    }

    // Clear the vector
    a.clear();

    // The vector must be empty now
    testAssert(a.count() == 0);
    testAssert(a.size() == size);

    for (Size i = 0; i < size; i++)
    {
        testAssert(a.get(i) == ZERO);
    }
    return OK;
}

TestCase(VectorCompare)
{
    TestData<uint> udata;
    TestData<int> idata;
    uint size  = udata.uvalue(256, 128);
    Vector<int> a1(size), a2(size);

    // Fill the vectors completely
    for (Size i = 0; i < size; i++)
    {
        // Generate value and insert it at the current index
        a1.put(i, idata.value());
        a2.put(i, idata[i]);
    }

    // Both vectors must be equal
    testAssert(a1.compareTo(a2) == 0);
    testAssert(a1.equals(a2));
    testAssert(a1.size() == a2.size());
    testAssert(a1.count() == a2.count());

    // Change one item. Vectors cannot be equal
    a1.put(0, ~(a1[0]));
    testAssert(a1.compareTo(a2) != 0);
    testAssert(!a1.equals(a2));
    testAssert(a1.size() == a2.size());
    testAssert(a1.count() == a2.count());

    // Remove one item. Vectors cannot be equal
    a1.put(0, ~(a1[0]));
    a1.remove(0);
    testAssert(a1.compareTo(a2) != 0);
    testAssert(!a1.equals(a2));
    testAssert(a1.size() == a2.size());
    testAssert(a1.count() != a2.count());
    return OK;
}
