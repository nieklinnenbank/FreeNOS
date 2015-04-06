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
#include <ListIterator.h>

TestCase(ListIteratorConstruct)
{
    List<int> lst;
    ListIterator<int> i(lst);

    // The iterator should be empty
    testAssert(!i.hasNext());
    testAssert(!i.hasCurrent());
    return OK;
}

TestCase(ListIteratorCurrent)
{
    List<int> lst;
    TestInt<int> ints(INT_MIN, INT_MAX);
    TestInt<Size> sizes(32, 64);
    Size size = sizes.random();

    // Generate a random list of items
    for (Size i = 0; i < size; i++)
        lst.append(ints.random());

    // Iterate the list via hasCurrent() and current()
    ListIterator<int> it(lst);

    for (Size i = 0; i < size; i++, it++)
    {
        testAssert(it.hasCurrent());
        testAssert(it.current() == ints[i]);
    }
    return OK;
}

TestCase(ListIteratorNext)
{
    List<int> lst;
    TestInt<int> ints(INT_MIN, INT_MAX);
    TestInt<Size> sizes(32, 64);
    Size size = sizes.random();

    // Generate a random list of items
    for (Size i = 0; i < size; i++)
        lst.append(ints.random());

    // Iterate the list via hasNext() and next()
    ListIterator<int> it(lst);

    for (Size i = 0; i < size; i++)
    {
        testAssert(it.hasNext());
        testAssert(it.next() == ints[i]);
        testAssert(it.current() == ints[i]);
    }
    return OK;
}

TestCase(ListIteratorRemoveFirst)
{
    List<int> lst;
    TestInt<int> ints(INT_MIN, INT_MAX);
    TestInt<Size> sizes(32, 64);
    Size size = sizes.random();

    // Generate a random list of items
    for (Size i = 0; i < size; i++)
        lst.append(ints.random());

    // Start iterating on the list.
    ListIterator<int> it(lst);

    // Remove the first item.
    testAssert(it.remove());
    testAssert(it.m_next == lst.head());
    testAssert(it.m_current == lst.head());
    testAssert(lst.head()->data == ints[1]);
    testAssert(lst.first() == ints[1]);
    testAssert(lst.count() == size-1);

    // Check that the iterator can iterate the rest of the list.
    for (Size i = 1; i < size; i++, it++)
    {
        testAssert(it.hasCurrent());
        testAssert(it.current() == ints[i]);
    }
    return OK;
}

TestCase(ListIteratorRemoveLast)
{
    List<int> lst;
    TestInt<int> ints(INT_MIN, INT_MAX);
    TestInt<Size> sizes(32, 64);
    Size size = sizes.random();

    // Generate a random list of items
    for (Size i = 0; i < size; i++)
        lst.append(ints.random());

    // Move iterator to the last item.
    ListIterator<int> it(lst);
    for (Size i = 0; i < size - 1; i++)
        it++;

    // Remove the last item.
    testAssert(it.remove());
    testAssert(it.m_next == ZERO);
    testAssert(it.m_current == ZERO);
    testAssert(lst.head()->data == ints[0]);
    testAssert(lst.tail()->data == ints[size-2]);
    testAssert(lst.first() == ints[0]);
    testAssert(lst.last() == ints[size-2]);
    testAssert(lst.count() == size-1);

    // The iterator should stop now.
    testAssert(!it.hasNext());
    testAssert(!it.hasCurrent());

    // Check that the iterator can iterate again.
    it.reset();
    for (Size i = 0; i < size-1; i++, it++)
    {
        testAssert(it.hasCurrent());
        testAssert(it.current() == ints[i]);
    }
    return OK;
}

TestCase(ListIteratorRemoveSingle)
{
    List<int> lst;
    TestInt<int> ints(INT_MIN, INT_MAX);
    TestInt<Size> sizes(32, 64);
    Size size = sizes.random();
    Size position = size/2;

    // Generate a random list of items
    for (Size i = 0; i < size; i++)
        lst.append(ints.random());

    // Start iterating.
    ListIterator<int> it(lst);
    for (Size i = 0; i < size - 1; i++, it++)
    {
        if (i == position)
        {
            // Remove one item.
            testAssert(it.remove());
            testAssert(it.m_next != ZERO);
            testAssert(it.m_current != ZERO);
            testAssert(it.current() == ints[i+1]);

            // Check the list administration
            testAssert(lst.head()->data == ints[0]);
            testAssert(lst.tail()->data == ints[size-1]);
            testAssert(lst.first() == ints[0]);
            testAssert(lst.last() == ints[size-1]);
            testAssert(lst.count() == size-1);
        }
        testAssert(it.current() == ints[ i >= position ? i+1 : i ]);
    }
    // The iterator should stop now.
    testAssert(!it.hasNext());
    testAssert(!it.hasCurrent());

    // Check that the iterator can iterate again.
    it.reset();
    for (Size i = 0; i < size-1; i++)
    {
        // Skip the removed item.
        if (i != position)
        {
            testAssert(it.hasCurrent());
            testAssert(it.current() == ints[i]);
            it++;
        }
    }
    return OK;
}

TestCase(ListIteratorRemoveAll)
{
    List<int> lst;
    TestInt<int> ints(INT_MIN, INT_MAX);
    TestInt<Size> sizes(32, 64);
    Size size = sizes.random();

    // Generate a random list of items
    for (Size i = 0; i < size; i++)
        lst.append(ints.random());

    // Remove all items by iteration.
    ListIterator<int> it(lst);
    for (; it.hasCurrent();)
        it.remove();

    // The iterator should be empty.
    testAssert(!it.hasNext());
    testAssert(!it.hasCurrent());

    // The list should be empty.
    testAssert(lst.isEmpty());
    testAssert(lst.count() == 0);
    testAssert(lst.size() == 0);
    testAssert(lst.head() == ZERO);
    testAssert(lst.tail() == ZERO);
    return OK;
}

TestCase(ListIteratorReset)
{
    List<int> lst;
    TestInt<int> ints(INT_MIN, INT_MAX);
    TestInt<Size> sizes(32, 64);
    Size size = sizes.random();
    Size rounds = sizes.random();

    // Generate a random list of items
    for (Size i = 0; i < size; i++)
        lst.append(ints.random());

    // Perform a random number of iteration rounds
    for (Size i = 0; i < rounds; i++)
    {
        // Iterate the list via hasCurrent() and current()
        ListIterator<int> it(lst);

        // At a random position while iterating, perform a reset.
        Size position = sizes.random();

        for (Size j = 0; j < size; j++, it++)
        {
            testAssert(it.hasCurrent());
            testAssert(it.current() == ints[j]);

            // Perform a reset?
            if (j == position)
                break;
        }
        // Reset the iterator.
        it.reset();

        // Now iterate again from the start to the end.
        for (Size j = 0; j < size; j++, it++)
        {
            testAssert(it.hasCurrent());
            testAssert(it.current() == ints[j]);
        }
    }
    return OK;
}
