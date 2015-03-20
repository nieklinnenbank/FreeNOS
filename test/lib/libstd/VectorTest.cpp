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
    TestData<uint> sizes(64, 16);
    Vector<int> a(sizes.random());

    // Check the vector is empty with the correct size
    testAssert(a.size() == sizes[0]);
    testAssert(a.count() == 0);
    return OK;
}

TestCase(VectorOfStrings)
{
    return SKIP;
}

TestCase(VectorFill)
{
    TestData<uint> sizes(64, 16);
    TestData<int> ints(INT_MAX, INT_MIN);
    Vector<int> a(sizes.random());

    // Fill the vector with a random value
    a.fill(ints.random());

    // Check the vector is completely filled with the value
    for (Size i = 0; i < a.count(); i++)
    {
        testAssert(a[i] == ints[0]);
    }
    // Administration should be correct
    testAssert(a.size() == sizes[0]);
    testAssert(a.count() == sizes[0]);
    return OK;
}

TestCase(VectorResize)
{
    TestData<uint> sizes(64, 16);
    TestData<int> ints(INT_MAX, INT_MIN);
    Vector<int> a(sizes.random());

    // First completely fill the vector three times its original size.
    for (Size i = 0; i < (sizes[0] * 3); i++)
    {
        testAssert(a.insert(ints.random()) == i);
        testAssert(a.at(i) == ints[i]);
    }
    // Check administration counters
    testAssert(a.count() == sizes[0] * 3);
    testAssert(a.size() >= sizes[0] * 3);
    return OK;
}

TestCase(VectorPutMultiple)
{
    TestData<uint> sizes(256, 128);
    TestData<int> ints(INT_MAX, INT_MIN);
    Vector<int> a(sizes.random());

    // Insert one value.
    a.insert(0, ints.random());
    testAssert(a[0] == ints[0]);

    // Insert another value on the same index.
    // It must overwrite but keep administration correct.
    a.insert(0, ints.random());
    testAssert(a[0] == ints[1]);
    testAssert(a.count() == 1);
    testAssert(a.size() == sizes[0]);
    return OK;
}

TestCase(VectorPutSeq)
{
    TestData<uint> sizes(256, 128);
    TestData<int> ints(INT_MAX, INT_MIN);
    Vector<int> a(sizes.random());

    // Clear the vector
    a.fill(0);

    // Fill the vector sequentially
    for (Size i = 0; i < sizes[0]; i++)
    {
        // Generate value and insert it at the current index
        a.insert(i, ints.random());

        // Assert that all previously inserted values are still there
        // and all other places are ZERO.
        for (Size j = 0; j < sizes[0]; j++)
        {
            if (j <= i)
            {
                testAssert(a[j] == ints[j]);
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
    TestData<uint> sizes(255, 0);
    TestData<uint> indexes(255, 0);
    TestData<int> ints(INT_MAX, INT_MIN);
    Vector<int> a(256);
    uint count = sizes.random();

    // Retrieve random indexes
    indexes.unique(256);

    // Clear the vector
    a.fill(0);

    // Fill the vector randomly
    for (Size i = 0; i < count; i++)
    {
        // Generate value and insert it at a random index
        a.insert(indexes[i], ints.random());

        // Assert that all previously inserted values are still there
        // and that empty places are ZERO.
        for (Size j = 0; j < 256; j++)
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
    return OK;
}

TestCase(VectorRemoveOne)
{
    TestData<uint> sizes(256, 128);
    TestData<int> ints(INT_MAX, INT_MIN);
    Vector<int> a(sizes.random());
    TestData<uint> indexes(sizes[0], 0);

    // Completely fill the vector sequentially
    for (Size i = 0; i < sizes[0]; i++)
        // Generate value and insert it at the current index
        a.insert(i, ints.random());

    // Remove one item randomly from the vector
    testAssert(a.remove(indexes.random()));
    testAssert(a.count() == sizes[0] - 1);
    testAssert(a.size() == sizes[0]);

    // Removing non-existing item should fail.
    testAssert(!a.remove(sizes[0]));
    testAssert(!a.remove(sizes[0] + sizes.random()));

    // Check that all items before the item are still inside, except the item removed.
    for (Size i = 0; i < indexes[0]; i++)
    {
        testAssert(a[i] == ints[i]);
    }
    // After the removed item.
    for (Size i = indexes[0]; i < sizes[0] - 1; i++)
    {
        testAssert(a[i] == ints[i+1]);
    }
    return OK;
}

TestCase(VectorClear)
{
    TestData<uint> sizes(256, 128);
    TestData<int> ints(INT_MAX, INT_MIN);
    Vector<int> a(sizes.random());

    // Fill the vector completely
    for (Size i = 0; i < sizes[0]; i++)
    {
        // Generate value and insert it at the current index
        a.insert(i, ints.random());
    }

    // Clear the vector
    a.clear();

    // The vector must be empty now
    testAssert(a.count() == 0);
    testAssert(a.size() == sizes[0]);

    for (Size i = 0; i < sizes[0]; i++)
    {
        testAssert(a.get(i) == ZERO);
    }
    return OK;
}

TestCase(VectorCompare)
{
    TestData<uint> sizes(256, 128);
    TestData<int> ints(INT_MAX, INT_MIN);
    Vector<int> a1(sizes.random());
    Vector<int> a2(sizes[0]);

    // Fill the vectors completely
    for (Size i = 0; i < sizes[0]; i++)
    {
        // Generate value and insert it at the current index
        a1.insert(i, ints.random());
        a2.insert(i, ints[i]);
    }

    // Both vectors must be equal
    testAssert(a1.compareTo(a2) == 0);
    testAssert(a1.equals(a2));
    testAssert(a1.size() == a2.size());
    testAssert(a1.count() == a2.count());

    // Change one item. Vectors cannot be equal
    a1.insert(0, ~(a1[0]));
    testAssert(a1.compareTo(a2) != 0);
    testAssert(!a1.equals(a2));
    testAssert(a1.size() == a2.size());
    testAssert(a1.count() == a2.count());

    // Remove one item. Vectors cannot be equal
    a1.insert(0, ~(a1[0]));
    a1.remove(0);
    testAssert(a1.compareTo(a2) != 0);
    testAssert(!a1.equals(a2));
    testAssert(a1.size() == a2.size());
    testAssert(a1.count() != a2.count());
    return OK;
}
