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

#include <TestRunner.h>
#include <TestInt.h>
#include <TestCase.h>
#include <TestMain.h>
#include <Channel.h>

class DummyChannel : public Channel
{
    virtual Channel::Result read(void *buffer) { return Channel::IOError; }
    virtual Channel::Result write(void *buffer) { return Channel::IOError; }
    virtual Channel::Result flush() { return Channel::IOError; }
};

TestCase(ChannelConstruct)
{
    DummyChannel ch;

    testAssert(ch.m_mode == Channel::Consumer);
    testAssert(ch.getMessageSize() == 0);
    testAssert(ch.getMaximumMessages() == 0);

    return OK;
}

TestCase(ChannelMode)
{
    DummyChannel ch;

    testAssert(ch.m_mode == Channel::Consumer);
    testAssert(ch.setMode(Channel::Producer) == Channel::Success);
    testAssert(ch.m_mode == Channel::Producer);

    return OK;
}

TestCase(ChannelMessageSize)
{
    DummyChannel ch;

    testAssert(ch.getMessageSize() == 0);
    testAssert(ch.setMessageSize(32) == Channel::Success);
    testAssert(ch.getMessageSize() == 32);
    testAssert(ch.m_messageSize == 32);
    testAssert(ch.m_maximumMessages == 0);

    return OK;
}
