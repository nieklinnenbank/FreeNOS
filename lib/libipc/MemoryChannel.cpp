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

#include <Log.h>
#include <FreeNOS/System.h>
#include "MemoryChannel.h"

MemoryChannel::MemoryChannel()
    : Channel()
{
    MemoryBlock::set(&m_head, 0, sizeof(m_head));
}

MemoryChannel::Result MemoryChannel::setData(Address addr)
{
    if (m_messageSize < sizeof(RingHead) || m_messageSize > (PAGESIZE / 2))
        return InvalidArgument;

    if (m_data.map(addr, PAGESIZE) != IO::Success)
        return IOError;

    m_maximumMessages = (PAGESIZE / m_messageSize) - 1;

    if (m_mode == Producer)
        m_data.write(0, sizeof(m_head), &m_head);
    return Success;
}

MemoryChannel::Result MemoryChannel::setFeedback(Address addr)
{
    if (m_feedback.map(addr, PAGESIZE) != IO::Success)
        return IOError;

    if (m_mode == Consumer)
        m_feedback.write(0, sizeof(m_head), &m_head);
    return Success;
}

MemoryChannel::Result MemoryChannel::read(void *buffer)
{
    RingHead head;

    // busy wait until a message comes in
    while (true)
    {
        m_data.read(0, sizeof(head), &head);

        if (head.index != m_head.index)
            break;
    }
    // Read one message
    m_data.read((m_head.index+1) * m_messageSize, m_messageSize, buffer);

    // Increment head index
    m_head.index = (m_head.index + 1) % m_maximumMessages;

    // Update read index
    m_feedback.write(0, sizeof(m_head), &m_head);
    return Success;
}

MemoryChannel::Result MemoryChannel::write(void *buffer)
{
    RingHead reader;

    // Check for buffer space
    m_feedback.read(0, sizeof(RingHead), &reader);

    if (((m_head.index + 1) % m_maximumMessages) == reader.index)
    {
        ERROR("m_head.index=" << m_head.index << " maximum=" << m_maximumMessages << " reader.index=" << reader.index);
        return ChannelFull;
    }

    // write the message
    m_data.write((m_head.index+1) * m_messageSize, m_messageSize, buffer);

    // Increment write index
    m_head.index = (m_head.index + 1) % m_maximumMessages;
    m_data.write(0, sizeof(m_head), &m_head);
    return Success;
}
