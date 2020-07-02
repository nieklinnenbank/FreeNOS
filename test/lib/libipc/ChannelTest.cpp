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
  public:
    DummyChannel(const Channel::Mode m, const Size sz) : Channel(m, sz) { }
    virtual Channel::Result read(void *buffer) { return Channel::IOError; }
    virtual Channel::Result write(const void *buffer) { return Channel::IOError; }
    virtual Channel::Result flush() { return Channel::IOError; }
};

TestCase(ChannelConstruct)
{
    DummyChannel ch(Channel::Consumer, sizeof(u32));

    testAssert(ch.m_mode == Channel::Consumer);
    testAssert(ch.m_messageSize == sizeof(u32));

    return OK;
}

TestCase(ChannelMessageSize)
{
    DummyChannel ch(Channel::Consumer, sizeof(u32));

    testAssert(ch.getMessageSize() == sizeof(u32));

    return OK;
}
