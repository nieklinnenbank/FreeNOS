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
#include <List.h>

TestCase(ListConstruct)
{
    List<int> lst;

    // The list should be empty
    testAssert(lst.count() == 0);
    testAssert(lst.size() == 0);
    testAssert(lst.count() == lst.size());
    testAssert(lst.head() == ZERO);
    testAssert(lst.tail() == ZERO);
    testAssert(lst.isEmpty());
    return OK;
}

TestCase(ListOfStrings)
{
    return SKIP;
}

TestCase(ListPrependEmpty)
{
    List<int> lst;
    TestData<int> ints(INT_MAX, INT_MIN);

    // Prepend an item to an empty list.
    lst.prepend(ints.random());

    // The new item should be the only item on the list.
    // Additionally, it should be both head and tail of the list.
    testAssert(lst.first() == ints[0]);
    testAssert(lst.last() == ints[0]);
    testAssert(lst[0] == ints[0]);
    testAssert(lst.head() == lst.tail());
    testAssert(lst.count() == 1);
    testAssert(lst.size() == 1);
    testAssert(!lst.isEmpty());

    // Check head
    testAssert(lst.head()->data == ints[0]);
    testAssert(lst.head()->prev == ZERO);
    testAssert(lst.head()->next == ZERO);

    // Check tail
    testAssert(lst.tail()->data == ints[0]);
    testAssert(lst.tail()->prev == ZERO);
    testAssert(lst.tail()->next == ZERO);

    return OK;
}

TestCase(ListPrepend)
{
    List<int> lst;
    TestData<int> ints(INT_MAX, INT_MIN);
    TestData<Size> sizes(64, 32);
    Size size = sizes.random();

    // Generate a random list of items
    for (Size i = 0; i < size; i++)
    {
        lst.prepend(ints.random());

        // Check that the item was added at the head.
        testAssert(lst[0] == ints[i]);

        // Check counters
        testAssert(lst.count() == i+1);
        testAssert(lst.size() == i+1);

        // Check head
        testAssert(lst.head() != ZERO);
        testAssert(lst.head()->prev == ZERO);
        testAssert(lst.head()->data == ints[i]);

        // Check tail
        testAssert(lst.tail() != ZERO);
        testAssert(lst.tail()->next == ZERO);
        testAssert(lst.tail()->data == ints[0]);

        // Check whole list contents.
        // See that all items already inserted earlier are there.
        List<int>::Node *node = lst.head();
        for (Size j = 0; j < i+1; j++, node = node->next)
        {
            testAssert(node->data == ints[i-j]);

            // Check previous/next pointer integrity.
            testAssert(!node->prev || node->prev->next == node);
            testAssert(!node->next || node->next->prev == node);

            // Previous pointer cannot be zero except for head.
            testAssert(j == 0 || node->prev != ZERO);

            // Next pointer cannot be zero except for tail.
            testAssert(j == i || node->next != ZERO);
        }
    }
    return OK;
}

TestCase(ListAppendEmpty)
{
    List<int> lst;
    TestData<int> ints(INT_MAX, INT_MIN);

    // Append an item to an empty list.
    lst.append(ints.random());

    // The new item should be the only item on the list.
    // Additionally, it should be both head and tail of the list.
    testAssert(lst.first() == ints[0]);
    testAssert(lst.last() == ints[0]);
    testAssert(lst[0] == ints[0]);
    testAssert(lst.head() == lst.tail());
    testAssert(lst.count() == 1);
    testAssert(lst.size() == 1);
    testAssert(!lst.isEmpty());

    // Check head
    testAssert(lst.head()->data == ints[0]);
    testAssert(lst.head()->prev == ZERO);
    testAssert(lst.head()->next == ZERO);

    // Check tail
    testAssert(lst.tail()->data == ints[0]);
    testAssert(lst.tail()->prev == ZERO);
    testAssert(lst.tail()->next == ZERO);

    return OK;
}

TestCase(ListAppend)
{
    List<int> lst;
    TestData<int> ints(INT_MAX, INT_MIN);
    TestData<Size> sizes(64, 32);
    Size size = sizes.random();

    // Generate a random list of items
    for (Size i = 0; i < size; i++)
    {
        lst.append(ints.random());

        // Check that the item was added at the tail.
        testAssert(lst[i] == ints[i]);

        // Check counters
        testAssert(lst.count() == i+1);
        testAssert(lst.size() == i+1);

        // Check head
        testAssert(lst.head() != ZERO);
        testAssert(lst.head()->prev == ZERO);
        testAssert(lst.head()->data == ints[0]);

        // Check tail
        testAssert(lst.tail() != ZERO);
        testAssert(lst.tail()->next == ZERO);
        testAssert(lst.tail()->data == ints[i]);

        // Check whole list contents.
        // See that all items already inserted earlier are there.
        List<int>::Node *node = lst.head();
        for (Size j = 0; j < i+1; j++, node = node->next)
        {
            testAssert(node->data == ints[j]);

            // Check previous/next pointer integrity.
            testAssert(!node->prev || node->prev->next == node);
            testAssert(!node->next || node->next->prev == node);

            // Previous pointer cannot be zero except for head.
            testAssert(j == 0 || node->prev != ZERO);

            // Next pointer cannot be zero except for tail.
            testAssert(j == i || node->next != ZERO);
        }
    }
    return OK;
}

TestCase(ListRemoveFirst)
{
    List<int> lst;
    TestData<int> ints(INT_MAX, INT_MIN);
    Size size = 64;

    // Generate unique values
    ints.unique(size);

    // Fiil the list with the values.
    for (Size i = 0; i < size; i++)
        lst.append(ints[i]);

    // Check initial size.
    testAssert(lst.size() == size);
    testAssert(lst.count() == size);

    // Now remove the first item at the head.
    testAssert(lst.remove(ints[0]) == 1);
    testAssert(lst.count() == size - 1);
    testAssert(lst.size() == size - 1);

    // Check head
    testAssert(lst.head() != ZERO);
    testAssert(lst.head()->prev == ZERO);
    testAssert(lst.head()->data == ints[1]);

    // Check tail
    testAssert(lst.tail() != ZERO);
    testAssert(lst.tail()->next == ZERO);
    testAssert(lst.tail()->data == ints[size-1]);

    // Check that all other items are still on the list.
    List<int>::Node *node = lst.head();
    for (Size i = 1; i < size; i++, node = node->next)
    {
        testAssert(node->data == ints[i]);

        // Check previous/next pointer integrity.
        testAssert(!node->prev || node->prev->next == node);
        testAssert(!node->next || node->next->prev == node);

        // Previous pointer cannot be zero except for head.
        testAssert(i == 1 || node->prev != ZERO);

        // Next pointer cannot be zero except for tail.
        testAssert(i == size-1 || node->next != ZERO);
    }
    return OK;
}

TestCase(ListRemoveLast)
{
    List<int> lst;
    TestData<int> ints(INT_MAX, INT_MIN);
    Size size = 64;

    // Generate unique values
    ints.unique(size);

    // Fiil the list with the values.
    for (Size i = 0; i < size; i++)
        lst.append(ints[i]);

    // Check initial size.
    testAssert(lst.size() == size);
    testAssert(lst.count() == size);

    // Now remove the last item at the tail.
    testAssert(lst.remove(ints[size-1]) == 1);
    testAssert(lst.count() == size - 1);
    testAssert(lst.size() == size - 1);

    // Check head
    testAssert(lst.head() != ZERO);
    testAssert(lst.head()->prev == ZERO);
    testAssert(lst.head()->data == ints[0]);

    // Check tail
    testAssert(lst.tail() != ZERO);
    testAssert(lst.tail()->next == ZERO);
    testAssert(lst.tail()->data == ints[size-2]);

    // Check that all other items are still on the list.
    List<int>::Node *node = lst.head();
    for (Size i = 0; i < size-1; i++, node = node->next)
    {
        testAssert(node->data == ints[i]);

        // Check previous/next pointer integrity.
        testAssert(!node->prev || node->prev->next == node);
        testAssert(!node->next || node->next->prev == node);

        // Previous pointer cannot be zero except for head.
        testAssert(i == 0 || node->prev != ZERO);

        // Next pointer cannot be zero except for tail.
        testAssert(i == size-2 || node->next != ZERO);
    }
    return OK;
}

TestCase(ListRemoveSingle)
{
    List<int> lst;
    TestData<int> ints(INT_MAX, INT_MIN);
    Size size = 64;

    // Generate unique values
    ints.unique(size);

    // Fiil the list with the values.
    for (Size i = 0; i < size; i++)
        lst.append(ints[i]);

    // Check initial size.
    testAssert(lst.size() == size);
    testAssert(lst.count() == size);

    // Now remove one item in the middle of the list.
    testAssert(lst.remove(ints[size/2]) == 1);
    testAssert(lst.count() == size - 1);
    testAssert(lst.size() == size - 1);

    // Check head
    testAssert(lst.head() != ZERO);
    testAssert(lst.head()->prev == ZERO);
    testAssert(lst.head()->data == ints[0]);

    // Check tail
    testAssert(lst.tail() != ZERO);
    testAssert(lst.tail()->next == ZERO);
    testAssert(lst.tail()->data == ints[size-1]);

    // Check that all other items are still on the list.
    List<int>::Node *node = lst.head();
    for (Size i = 0; i < size; i++, node = node->next)
    {
        // Skip the removed item.
        if (i == size/2)
            i++;
        testAssert(node->data == ints[i]);

        // Check previous/next pointer integrity.
        testAssert(!node->prev || node->prev->next == node);
        testAssert(!node->next || node->next->prev == node);

        // Previous pointer cannot be zero except for head.
        testAssert(i == 0 || node->prev != ZERO);

        // Next pointer cannot be zero except for tail.
        testAssert(i == size-1 || node->next != ZERO);
    }
    return OK;
}

TestCase(ListRemoveMultiple)
{
    List<int> lst;
    TestData<int> ints(INT_MAX, INT_MIN);
    TestData<uint> sizes(32, 16);
    Size size = 64;
    Size extra = sizes.random();

    // Generate unique values
    ints.unique(size);

    // Fill the list with the values.
    for (Size i = 0; i < size; i++)
        lst.append(ints[i]);

    // Add extra values to the list.
    // Repeat the first value a random number of times.
    for (Size i = 0; i < extra; i++)
        lst.append(ints[0]);

    // Check initial size.
    testAssert(lst.size() == size + extra);
    testAssert(lst.count() == size + extra);

    // Now remove multiple items from the list.
    testAssert(lst.remove(ints[0]) == (int) extra + 1);
    testAssert(lst.count() == size - 1);
    testAssert(lst.size() == size - 1);

    // Check head
    testAssert(lst.head() != ZERO);
    testAssert(lst.head()->prev == ZERO);
    testAssert(lst.head()->data == ints[1]);

    // Check tail
    testAssert(lst.tail() != ZERO);
    testAssert(lst.tail()->next == ZERO);
    testAssert(lst.tail()->data == ints[size-1]);

    // Check that all other items are still on the list.
    List<int>::Node *node = lst.head();
    for (Size i = 1; i < size; i++, node = node->next)
    {
        testAssert(node->data == ints[i]);

        // Check previous/next pointer integrity.
        testAssert(!node->prev || node->prev->next == node);
        testAssert(!node->next || node->next->prev == node);

        // Previous pointer cannot be zero except for head.
        testAssert(i == 1 || node->prev != ZERO);

        // Next pointer cannot be zero except for tail.
        testAssert(i == size-1 || node->next != ZERO);
    }
    return OK;
}

TestCase(ListContains)
{
    List<int> lst;
    TestData<int> ints(INT_MAX, INT_MIN);
    TestData<Size> sizes(64, 32);
    Size size = sizes.random();

    // Generate unique values
    ints.unique(size);

    // Add the items to the list, except the last one.
    for (Size i = 0; i < size-1; i++)
        lst.append(ints[i]);

    // Check that all items are on the list using contains().
    for (Size i = 0; i < size-1; i++)
        testAssert(lst.contains(ints[i]));

    // Check that the last one is not on the list using contains().
    testAssert(!lst.contains(ints[size-1]));
    return OK;
}

TestCase(ListGet)
{
    List<int> lst;
    TestData<int> ints(INT_MAX, INT_MIN);
    TestData<Size> sizes(64, 32);
    Size size = sizes.random();

    // Generate a random list of items
    for (Size i = 0; i < size; i++)
    {
        lst.append(ints.random());

        // Try to retrieve each item on the list.
        for (Size j = 0; j < i+1; j++)
        {
            // Retrieve reference and by-value.
            testAssert(lst[j] == ints[j]);
            testAssert(lst.at(j) == ints[j]);
            testAssert(lst.value(j) == ints[j]);

            // The get() function should return a valid pointer for
            // and existing item, and ZERO for non-existing.
            testAssert(lst.get(j) != ZERO);
            testAssert(*lst.get(j) == ints[j]);
            testAssert(lst.get(size) == ZERO);
            testAssert(lst.get(size+1) == ZERO);
        }
    }

    // Test first() and last()
    testAssert(lst.first() == ints[0]);
    testAssert(lst.last() == ints[size-1]);
    testAssert(!lst.isEmpty())
    return OK;
}

TestCase(ListClear)
{
    List<int> lst;
    TestData<int> ints(INT_MAX, INT_MIN);
    TestData<Size> sizes(64, 32);
    Size size = sizes.random();

    // Add random list of items to the list.
    for (Size i = 0; i < size; i++)
        lst.append(ints.random());

    // Clear the list
    lst.clear();

    // The list should be completely empty now
    testAssert(lst.count() == 0);
    testAssert(lst.size() == 0);
    testAssert(lst.count() == lst.size());
    testAssert(lst.head() == ZERO);
    testAssert(lst.tail() == ZERO);
    testAssert(lst.isEmpty());
    return OK;
}
