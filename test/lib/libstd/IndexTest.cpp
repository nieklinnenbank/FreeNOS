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
#include <TestInt.h>
#include <TestMain.h>
#include <Index.h>

TestCase(IndexConstruct)
{
    const Size sz = 128;
    Index<String, sz> index;

    // Index must be empty after construction
    testAssert(index.count() == 0);
    testAssert(index.size() == sz);

    for (Size i = 0; i < sz; i++)
    {
        testAssert(index.m_array[i] == ZERO);
    }

    return OK;
}

TestCase(IndexInsert)
{
    const Size sz = 128;
    Index<String, sz> index;
    String mystring("test");
    String mystring2("test2");
    String mystring3("test3");
    Size idx = 0;

    // Try to insert beyond the array range
    testAssert(index.insertAt(sz, &mystring) == false);
    testAssert(index.insertAt(sz + 1, &mystring) == false);

    // ZERO object pointer not allowed
    testAssert(index.insert(idx, ZERO) == false);

    // Insert to the first available spot
    testAssert(index.insert(idx, &mystring) == true);
    testAssert(idx == 0);
    testAssert(index.m_array[0] == &mystring);
    testAssert(index.count() == 1);
    testAssert(index.size() == sz);

    // Insert a second object, which should come directly after the first one
    testAssert(index.insert(idx, &mystring2) == true);
    testAssert(idx == 1);
    testAssert(index.m_array[0] == &mystring);
    testAssert(index.m_array[1] == &mystring2);
    testAssert(index.count() == 2);
    testAssert(index.size() == sz);

    // Now overwrite the second object
    testAssert(index.insertAt(1, &mystring3) == true);
    testAssert(index.m_array[0] == &mystring);
    testAssert(index.m_array[1] == &mystring3);
    testAssert(index.count() == 2);
    testAssert(index.size() == sz);

    return OK;
}

TestCase(IndexRemove)
{
    const Size sz = 128;
    Index<String, sz> index;
    Size idx = 0;
    String mystring("test");

    // Insert to the first available spot
    testAssert(index.insert(idx, &mystring) == true);
    testAssert(idx == 0);
    testAssert(index.m_array[0] == &mystring);
    testAssert(index.count() == 1);
    testAssert(index.size() == sz);

    // Try to remove non-existing object
    testAssert(index.remove(1) == false);

    // Position out of range
    testAssert(index.remove(sz) == false);
    testAssert(index.remove(sz + 1) == false);

    // Remove the object
    testAssert(index.remove(0) == true);
    testAssert(index.count() == 0);
    testAssert(index.size() == sz);

    return OK;
}

TestCase(IndexDeleteAll)
{
    const Size sz = 128;
    Index<String, sz> index;
    Size idx = 0;

    // Completely fill the index
    for (Size i = 0; i < sz; i++)
    {
        String *s = new String("test");
        testAssert(index.insert(idx, s) == true);
        testAssert(index.count() == i + 1);
        testAssert(index.size() == sz);
    }

    // Cannot add another item when full
    String tmp("obj");
    testAssert(index.insert(idx, &tmp) == false);

    // Now delete all objects
    index.deleteAll();

    // The Index must be empty now
    testAssert(index.count() == 0);
    testAssert(index.size() == sz);

    for (Size i = 0; i < sz; i++)
    {
        testAssert(index.m_array[i] == ZERO);
    }

    return OK;
}

TestCase(IndexContains)
{
    const Size sz = 128;
    Index<String, sz> index;
    Size idx = 0;
    String mystring("test");
    String otherstring("obj");

    // Insert to the first available spot
    testAssert(index.insert(idx, &mystring) == true);
    testAssert(idx == 0);
    testAssert(index.m_array[0] == &mystring);
    testAssert(index.count() == 1);
    testAssert(index.size() == sz);

    // Try to locate the item
    testAssert(index.contains(&mystring) == true);

    // Any other item is not found
    testAssert(index.contains(&otherstring) == false);
    return OK;
}
