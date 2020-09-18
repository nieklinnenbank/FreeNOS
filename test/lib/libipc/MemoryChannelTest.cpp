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

#include <FreeNOS/Constant.h>
#include <TestRunner.h>
#include <TestInt.h>
#include <TestCase.h>
#include <TestMain.h>
#include <MemoryChannel.h>

TestCase(MemoryChannelConstruct)
{
    MemoryChannel ch(Channel::Consumer, sizeof(u32));

    testAssert(ch.m_mode == Channel::Consumer);
    testAssert(ch.m_messageSize == sizeof(u32));
    testAssert(ch.m_head.index == 0);
    testAssert(ch.m_data.getBase() == IO_BASE);
    testAssert(ch.m_feedback.getBase() == IO_BASE);
    testAssert(ch.getMessageSize() == sizeof(u32));

    return OK;
}

TestCase(MemoryChannelSetVirtual)
{
    static u8 dataPageBuf[PAGESIZE];
    static u8 feedbackPageBuf[PAGESIZE];

    const Address dataPage = (Address) dataPageBuf;
    const Address feedbackPage = (Address) feedbackPageBuf;
    MemoryChannel cons(Channel::Consumer, sizeof(u32));
    MemoryChannel prod(Channel::Producer, sizeof(u32));

    // Default behaviour with hard reset
    testAssert(cons.setVirtual(dataPage, feedbackPage) == MemoryChannel::Success);
    testAssert(cons.m_data.getBase() == dataPage);
    testAssert(cons.m_feedback.getBase() == feedbackPage);
    testAssert(cons.m_head.index == ZERO);

    // Soft reset must restore the RingHead from the feedback page for consumer
    ((MemoryChannel::RingHead *) (feedbackPage))->index = 123;
    testAssert(cons.setVirtual(dataPage, feedbackPage, false) == MemoryChannel::Success);
    testAssert(cons.m_data.getBase() == dataPage);
    testAssert(cons.m_feedback.getBase() == feedbackPage);
    testAssert(cons.m_head.index == 123);

    // Soft reset must restore the RingHead from the data page for producer
    ((MemoryChannel::RingHead *) (dataPage))->index = 456;
    testAssert(prod.setVirtual(dataPage, feedbackPage, false) == MemoryChannel::Success);
    testAssert(prod.m_data.getBase() == dataPage);
    testAssert(prod.m_feedback.getBase() == feedbackPage);
    testAssert(prod.m_head.index == 456);

    return OK;
}

TestCase(MemoryChannelReadWriteSingle)
{
    static u32 dataPage[PAGESIZE / sizeof(u32)] = { 0 };
    static u32 feedbackPage[PAGESIZE / sizeof(u32)] = { 0 };
    TestInt<uint> writeValues(UINT_MIN, UINT_MAX);
    const u32 writeVal = writeValues.random();
    u32 readVal = 0;

    MemoryChannel prod(Channel::Producer, sizeof(u32));
    MemoryChannel cons(Channel::Consumer, sizeof(u32));

    // First assign pages
    testAssert(prod.setVirtual((const Address) &dataPage, (const Address) &feedbackPage) == MemoryChannel::Success);
    testAssert(cons.setVirtual((const Address) &dataPage, (const Address) &feedbackPage) == MemoryChannel::Success);

    // Write a single message
    testAssert(prod.write(&writeVal) == MemoryChannel::Success);

    // Verify data page contents. First slot has the RingHead.
    const MemoryChannel::RingHead *dataHead = (const MemoryChannel::RingHead *) &dataPage[0];
    testAssert(dataHead->index == 1);

    // Actual message is saved starting at position 1.
    testAssert(dataPage[1] == writeVal);

    // Rest of the data page is still zero
    for (Size i = 2; i < sizeof(dataPage) / sizeof(u32); i++)
    {
        testAssert(dataPage[i] == 0);
    }

    // Verify feedback page, which should be unchanged at this point
    const MemoryChannel::RingHead *feedbackHead = (const MemoryChannel::RingHead *) &feedbackPage[0];
    testAssert(feedbackHead->index == 0);

    // Read a single message
    testAssert(cons.read(&readVal) == MemoryChannel::Success);
    testAssert(readVal == writeVal);

    // Verify both headers point after the first message
    testAssert(dataHead->index == feedbackHead->index);
    testAssert(feedbackHead->index == 1);

    return OK;
}

TestCase(MemoryChannelReadWriteRandom)
{
    static u32 dataPage[PAGESIZE / sizeof(u32)] = { 0 };
    static u32 feedbackPage[PAGESIZE / sizeof(u32)] = { 0 };
    TestInt<uint> writeValues(UINT_MIN, UINT_MAX);
    TestInt<uint> countValues(16, 32);
    u32 readVal, idx = 0;

    MemoryChannel prod(Channel::Producer, sizeof(u32));
    MemoryChannel cons(Channel::Consumer, sizeof(u32));

    // First assign pages
    testAssert(prod.setVirtual((const Address) &dataPage, (const Address) &feedbackPage) == MemoryChannel::Success);
    testAssert(cons.setVirtual((const Address) &dataPage, (const Address) &feedbackPage) == MemoryChannel::Success);

    // Get ring header pointers
    const MemoryChannel::RingHead *dataHead = (const MemoryChannel::RingHead *) &dataPage[0];
    const MemoryChannel::RingHead *feedbackHead = (const MemoryChannel::RingHead *) &feedbackPage[0];

    // Write random amount of random messages
    for (Size i = 0; i < 16; i++)
    {
        const Size numMessages = countValues.random();

        // First write
        for (Size j = 0; j < numMessages; j++)
        {
            u32 writeVal = writeValues.random();
            testAssert(prod.write(&writeVal) == MemoryChannel::Success);
        }

        // Read all back out
        for (Size j = 0; j < numMessages; j++)
        {
            testAssert(cons.read(&readVal) == MemoryChannel::Success);
            testAssert(readVal == writeValues[idx++]);
        }

        // Channel is now empty again
        testAssert(feedbackHead->index == dataHead->index);
        testAssert(cons.read(&readVal) == MemoryChannel::NotFound);
    }

    return OK;
}

TestCase(MemoryChannelReadWriteEmpty)
{
    static u32 dataPage[PAGESIZE / sizeof(u32)] = { 0 };
    static u32 feedbackPage[PAGESIZE / sizeof(u32)] = { 0 };
    u32 readVal = 0;

    MemoryChannel prod(Channel::Producer, sizeof(u32));
    MemoryChannel cons(Channel::Consumer, sizeof(u32));

    // First assign pages
    testAssert(prod.setVirtual((const Address) &dataPage, (const Address) &feedbackPage) == MemoryChannel::Success);
    testAssert(cons.setVirtual((const Address) &dataPage, (const Address) &feedbackPage) == MemoryChannel::Success);

    // Initially the channel is empty
    testAssert(cons.read(&readVal) == MemoryChannel::NotFound);
    return OK;
}

TestCase(MemoryChannelReadWriteFull)
{
    static u32 dataPage[PAGESIZE / sizeof(u32)] = { 0 };
    static u32 feedbackPage[PAGESIZE / sizeof(u32)] = { 0 };
    TestInt<uint> writeValues(UINT_MIN, UINT_MAX);
    u32 readVal, writeVal;

    MemoryChannel prod(Channel::Producer, sizeof(u32));
    MemoryChannel cons(Channel::Consumer, sizeof(u32));

    // Maximum messages is minus 2, for the ringhead and index mechanism
    const Size maxMessages = (sizeof(dataPage) / sizeof(u32)) - 2U;

    // First assign pages
    testAssert(prod.setVirtual((const Address) &dataPage, (const Address) &feedbackPage) == MemoryChannel::Success);
    testAssert(cons.setVirtual((const Address) &dataPage, (const Address) &feedbackPage) == MemoryChannel::Success);

    // Get ring header pointers
    const MemoryChannel::RingHead *dataHead = (const MemoryChannel::RingHead *) &dataPage[0];
    const MemoryChannel::RingHead *feedbackHead = (const MemoryChannel::RingHead *) &feedbackPage[0];

    // Keep writing until the channel is completely filled
    for (Size i = 0; i < maxMessages; i++)
    {
        u32 writeVal = writeValues.random();
        testAssert(prod.write(&writeVal) == MemoryChannel::Success);

        // Verify data page contents. First slot has the RingHead.
        testAssert(dataHead->index == i + 1);
        testAssert(dataPage[i + 1] == writeVal);
    }

    // Attempt to write another message (must fail)
    testAssert(prod.write(&writeVal) == MemoryChannel::ChannelFull);

    // Verify feedback page, which should be unchanged at this point
    testAssert(feedbackHead->index == 0);

    // Now start reading out all messages until empty
    for (Size i = 0; i < maxMessages; i++)
    {
        testAssert(cons.read(&readVal) == MemoryChannel::Success);
        testAssert(readVal == writeValues[i]);
        testAssert(feedbackHead->index == i + 1);
    }

    // Channel is now empty again
    testAssert(feedbackHead->index == dataHead->index);
    testAssert(cons.read(&readVal) == MemoryChannel::NotFound);

    return OK;
}

TestCase(MemoryChannelFlush)
{
    static u32 dataPage[PAGESIZE / sizeof(u32)] = { 0 };
    static u32 feedbackPage[PAGESIZE / sizeof(u32)] = { 0 };

    MemoryChannel prod(Channel::Producer, sizeof(u32));
    MemoryChannel cons(Channel::Consumer, sizeof(u32));

    // First assign pages
    testAssert(prod.setVirtual((const Address) &dataPage, (const Address) &feedbackPage) == MemoryChannel::Success);
    testAssert(cons.setVirtual((const Address) &dataPage, (const Address) &feedbackPage) == MemoryChannel::Success);

    // Flushing must always succeed
    testAssert(cons.flush() == MemoryChannel::Success);
    testAssert(prod.flush() == MemoryChannel::Success);
    return OK;
}
