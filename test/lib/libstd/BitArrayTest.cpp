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
#include <BitArray.h>

TestCase(BitArrayConstruct)
{
    TestInt<Size> sizes(64, 128);
    BitArray ba(sizes.random());

    // Check that the Array has the right size and counters
    testAssert(ba.m_array != NULL);
    testAssert(ba.m_bitCount == sizes[0]);
    testAssert(ba.m_set  == 0);
    testAssert(ba.count(false) == sizes[0]);

    // It should only have zeroes
    for (Size i = 0; i < sizes[0]; i++)
        testAssert(!ba[i]);

    return OK;
}

TestCase(BitArraySetOne)
{
    TestInt<Size> indexes(0, 127);
    BitArray ba(128);

    // Set one bit to 1. Keep all others zero.
    ba.set(indexes.random(), true);
    testAssert(ba.isSet(indexes[0]));

    // Check that only one bit is 1, all others are zero.
    for (Size i = 0; i < 128; i++)
    {
        if (i != indexes[0])
        {
            testAssert(!ba.isSet(i));
        }
        else
        {
            testAssert(ba.isSet(i));
        }
    }
    // Unset the bit
    ba.set(indexes[0], false);
    testAssert(!ba.isSet(indexes[0]));
    return OK;
}

TestCase(BitArraySetRandom)
{
    TestInt<Size> indexes(0, 127);
    BitArray ba(128);

    // Generate a unique set of indexes
    indexes.unique(128);

    // Set a random number of bits to 1, at random indexes
    for (Size i = 0; i < 128; i++)
    {
        // Set and test the bit
        ba.set(indexes[i], true);
        testAssert(ba.isSet(indexes[0]));

        // Validate that all previous set bits are still 1
        // and that all other bits are zero
        for (Size j = 0; j < 128; j++)
        {
            if (j <= i)
            {
                testAssert(ba.isSet(indexes[j]));
            }
            else
            {
                testAssert(!ba.isSet(indexes[j]));
            }
        }
        // Validate counter
        testAssert(ba.count(true) == i + 1);
        testAssert(ba.count(false) == 128 - i - 1);
    }
    // All bits should be 1 now
    testAssert(ba.count(true) == 128);
    testAssert(ba.count(false) == 0);
    return OK;
}

TestCase(BitArraySetMultiple)
{
    BitArray ba(128);

    // Clear array
    ba.clear();

    // Check initial counters
    testAssert(ba.size() == 128);
    testAssert(ba.count(false) == 128);
    testAssert(ba.count(true) == 0);

    // Set the first bit multiple times
    for (Size i = 0; i < 10; i++)
        ba.set(0, true);

    // Check counters.
    testAssert(ba.size() == 128);
    testAssert(ba.count(true) == 1);
    testAssert(ba.count(false) == 127);

    // Now unset the first bit multiple times
    for (Size i = 0; i < 10; i++)
        ba.set(0, false);

    // Check counters
    testAssert(ba.size() == 128);
    testAssert(ba.count(false) == 128);
    testAssert(ba.count(true) == 0);
    return OK;
}

TestCase(BitArraySetRange)
{
    TestInt<Size> sizes(32, 64);
    Size size = sizes.random();
    TestInt<Size> indexes(0, 127-size);
    BitArray ba(128);
    Size idx = indexes.random();

    // Set a random range of bits at random offset to 1.
    ba.setRange(idx, idx + sizes[0]);

    // Check that only the bits inside the range are 1.
    for (Size i = 0; i < 128; i++)
    {
        if (i >= indexes[0] && i <= indexes[0] + sizes[0])
        {
            testAssert(ba.isSet(i));
        }
        else
        {
            testAssert(!ba.isSet(i));
        }
    }
    // Check administration counters
    testAssert(ba.count(true)  == sizes[0] + 1);
    testAssert(ba.count(false) == 128 - sizes[0] - 1);
    testAssert(ba.size() == 128);
    return OK;
}

TestCase(BitArraySetNextRandom)
{
    TestInt<Size> sizes(32, 64);
    Size size = sizes.random();
    TestInt<Size> indexes(0, 127-size);
    BitArray ba(128);
    Size bit;

    // Set a random number of bits at a random offset
    testAssert(ba.setNext(&bit, size, indexes.random()) == BitArray::Success);

    // Check that only the bits starting at the random index are set
    for (Size i = 0; i < 128; i++)
    {
        if (i >= indexes[0] && i < indexes[0] + size)
        {
            testAssert(ba[i]);
        }
        else
        {
            testAssert(!ba[i]);
        }
    }
    // Check administration
    testAssert(ba.count(true) == size);
    testAssert(ba.count(false) == 128 - size);
    testAssert(ba.size() == 128);
    return OK;
}

TestCase(BitArraySetNextOverwrite)
{
    TestInt<Size> indexes(0, 63);
    BitArray ba(128);
    Size bit;

    // Set half of the array to 1.
    for (Size i = 0; i < 64; i++)
        ba.set(i, true);

    // Now try to claim 32 unset bits. Start searching
    // anywhere in the first half of the array + 31bits.
    // setNext should always choose bit 64 until 96.
    testAssert(ba.setNext(&bit, 32, indexes.random()) == BitArray::Success);
    testAssert(bit == 64);

    // Always bits 64 until 96 should be choosen and set to 1.
    for (Size i = 0; i < 128; i++)
    {
        if (i < 96)
        {
            testAssert(ba[i]);
        }
        else
        {
            testAssert(!ba[i]);
        }
    }
    // Check administration
    testAssert(ba.count(true) == 64 + 32);
    testAssert(ba.count(false) == 32);
    testAssert(ba.size() == 128);
    return OK;
}

TestCase(BitArraySetNextExact)
{
    TestInt<Size> indexes(0, 128 - 32 - 1);
    BitArray ba(128);
    Size idx = indexes.random();
    Size bit;

    // Create a bitmap which has exactly the given number of bits unset.
    for (Size i = 0; i < 128; i++)
    {
        if (i >= idx && i < idx + 32)
        {
            ba.set(i, false);
        }
        else
        {
            ba.set(i, true);
        }
    }

    // Now let setNext run. It should take exactly the unset block.
    testAssert(ba.setNext(&bit, 32) == BitArray::Success);
    testAssert(bit == idx);

    // Now the whole bit array should be set.
    for (Size i = 0; i < 128; i++)
    {
        testAssert(ba[i]);
    }
    // Check administration
    testAssert(ba.count(true) == 128);
    testAssert(ba.count(false) == 0);
    testAssert(ba.size() == 128);
    return OK;
}

TestCase(BitArraySetNextOutOfMemory)
{
    BitArray ba(128);
    Size bit;

    // start offset out of range
    testAssert(ba.setNext(&bit, 4, 128) == BitArray::OutOfMemory);
    testAssert(ba.setNext(&bit, 4, 1000) == BitArray::OutOfMemory);

    // No bits available
    ba.setRange(0, 128);
    testAssert(ba.setNext(&bit, 4) == BitArray::OutOfMemory);
    return OK;
}

TestCase(BitArrayClear)
{
    TestInt<Size> indexes(0, 127);
    BitArray ba(128);

    // Generate a unique set of indexes
    indexes.unique(128);

    // Set some random bits
    for (Size i = 0; i < 64; i++)
        ba.set(indexes[i], true);

    // Now clear the bit array
    ba.clear();

    // The bit array must have only zeroes now
    for (Size i = 0; i < 128; i++)
    {
        testAssert(!ba[i]);
    }
    testAssert(ba.count(true) == 0);
    testAssert(ba.count(false) == 128);
    return OK;
}

TestCase(BitArraySetArray)
{
    TestInt<Size> indexes(0, 127);
    BitArray ba(128), ba2(128);

    // Generate a unique set of indexes
    indexes.unique(128);

    // Set some random bits
    for (Size i = 0; i < 64; i++)
        ba.set(indexes[i], true);

    // Assign the same array to ba2.
    ba2.setArray(ba.m_array);

    // Check administration
    testAssert(ba2.m_array == ba.m_array);
    testAssert(ba2.m_set   == ba.m_set);
    testAssert(ba2.m_bitCount  == ba.m_bitCount);
    testAssert(!ba2.m_allocated);
    return OK;
}
