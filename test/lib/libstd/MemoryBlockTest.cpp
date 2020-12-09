/*
 * Copyright (C) 2020 Niek Linnenbank
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
#include <TestMain.h>
#include <TestInt.h>
#include <Macros.h>
#include <MemoryBlock.h>

TestCase(InitData)
{
    u8 array1[256];
    TestInt<uint> ints(0, 0xff);

    MemoryBlock::set(array1, ints.random(), sizeof(array1));

    for (Size i = 0; i < sizeof(array1); i++)
    {
        testAssert(array1[i] == ints[0]);
    }

    return OK;
}

TestCase(CopyData)
{
    TestInt<uint> ints(0, 0xff);
    u8 array1[256];
    u8 array2[256];

    // Clear both arrays first
    MemoryBlock::set(array1, 0, sizeof(array1));
    MemoryBlock::set(array2, 0, sizeof(array2));

    // Fill the first array with random integers
    for (Size i = 0; i < sizeof(array1); i++)
    {
        array1[i] = ints.random();
    }

    // Copy the first array to the second
    MemoryBlock::copy(array2, array1, sizeof(array2));

    // Now verify both arrays are identical
    for (Size i = 0; i < sizeof(array1); i++)
    {
        testAssert(array1[i] == array2[i]);
        testAssert(array1[i] == ints[i]);
        testAssert(array2[i] == ints[i]);
    }

    return OK;
}

TestCase(CopyString)
{
    char str[128];

    MemoryBlock::copy(str, (char *)"testing", sizeof(str));
    testString(str, "testing");

    return OK;
}

TestCase(CompareData)
{
    TestInt<uint> ints(0, 0xff);
    u8 array1[256];
    u8 array2[256];

    // Fill the both arrays with random integers
    for (Size i = 0; i < sizeof(array1); i++)
    {
        array1[i] = ints.random();
        array2[i] = array1[i];
    }

    // The array are now identical
    testAssert(MemoryBlock::compare(&array1, &array2, sizeof(array1)));

    // Change one byte in the second array
    array2[0]++;
    testAssert(!MemoryBlock::compare(&array1, &array2, sizeof(array1)));

    return OK;
}

TestCase(CompareString)
{
    // Compare without specifying a maximum length
    testAssert(!MemoryBlock::compare("abc", "def"));
    testAssert(!MemoryBlock::compare("test", "testing"));
    testAssert(MemoryBlock::compare("test", "test"));

    // Compare with a maximum length
    testAssert(MemoryBlock::compare("test", "testing", 4));
    testAssert(MemoryBlock::compare("testasdf", "test1234", 4));
    testAssert(!MemoryBlock::compare("test123456", "test567890", 6));

    return OK;
}
