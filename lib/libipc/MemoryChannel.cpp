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

#include <Assert.h>
#include <Log.h>
#include <MemoryBlock.h>
#include "MemoryChannel.h"

MemoryChannel::MemoryChannel(const Channel::Mode mode, const Size messageSize)
    : Channel(mode, messageSize)
    , m_maximumMessages((PAGESIZE / messageSize) - 1U)
{
    assert(messageSize >= sizeof(RingHead));
    assert(messageSize < (PAGESIZE / 2));

    reset(true);
}

MemoryChannel::~MemoryChannel()
{
}

MemoryChannel::Result MemoryChannel::reset(const bool hardReset)
{
    if (hardReset)
    {
        MemoryBlock::set(&m_head, 0, sizeof(m_head));
    }
    else if (m_mode == Channel::Producer)
    {
        m_data.read(0, sizeof(m_head), &m_head);
    }
    else if (m_mode == Channel::Consumer)
    {
        m_feedback.read(0, sizeof(m_head), &m_head);
    }
    return Success;
}

MemoryChannel::Result MemoryChannel::setVirtual(const Address data,
                                                const Address feedback,
                                                const bool hardReset)
{
    m_data.setBase(data);
    m_feedback.setBase(feedback);

    return reset(hardReset);
}

MemoryChannel::Result MemoryChannel::setPhysical(const Address data,
                                                 const Address feedback,
                                                 const bool hardReset)
{
    Memory::Access dataAccess = Memory::User | Memory::Readable;
    Memory::Access feedAccess = Memory::User | Memory::Readable;

    switch (m_mode)
    {
        case Consumer:
            feedAccess |= Memory::Writable;
            break;

        case Producer:
            dataAccess |= Memory::Writable;
            break;
    }

    IO::Result result = m_data.map(data, PAGESIZE, dataAccess);
    if (result != IO::Success)
    {
        ERROR("failed to map data physical address " << (void*)data << ": " << (int)result);
        return IOError;
    }

    result = m_feedback.map(feedback, PAGESIZE, feedAccess);
    if (result != IO::Success)
    {
        ERROR("failed to map feedback physical address " << (void*)feedback << ": " << (int)result);
        return IOError;
    }

    return reset(hardReset);
}

MemoryChannel::Result MemoryChannel::unmap()
{
    Result result = Success;

    if (m_data.unmap() != IO::Success)
    {
        result = IOError;
    }

    if (m_feedback.unmap() != IO::Success)
    {
        result = IOError;
    }

    return result;
}

MemoryChannel::Result MemoryChannel::read(void *buffer)
{
    RingHead head;

    // Read the current ring head
    m_data.read(0, sizeof(head), &head);

    // Check if a message is present
    if (head.index == m_head.index)
        return NotFound;

    // Read one message
    m_data.read((m_head.index+1) * m_messageSize, m_messageSize, buffer);

    // Increment head index
    m_head.index = (m_head.index + 1) % m_maximumMessages;

    // Update read index
    m_feedback.write(0, sizeof(m_head), &m_head);
    return Success;
}

MemoryChannel::Result MemoryChannel::write(const void *buffer)
{
    RingHead reader;

    // Read current ring head
    m_feedback.read(0, sizeof(RingHead), &reader);

    // Check if buffer space is available for the message
    if (((m_head.index + 1) % m_maximumMessages) == reader.index)
        return ChannelFull;

    // write the message
    m_data.write((m_head.index+1) * m_messageSize, m_messageSize, buffer);

    // Increment write index
    m_head.index = (m_head.index + 1) % m_maximumMessages;
    m_data.write(0, sizeof(m_head), &m_head);
    return Success;
}

MemoryChannel::Result MemoryChannel::flush()
{
#ifndef INTEL
    if (m_mode == Producer)
        flushPage(m_data.getBase());
    else if (m_mode == Consumer)
        flushPage(m_feedback.getBase());
#endif /* INTEL */

    return Success;
}

MemoryChannel::Result MemoryChannel::flushPage(const Address page) const
{
    // Flush caches in usermode via the kernel.
    if (!isKernel)
    {
#ifndef __HOST__
        Memory::Range range;
        range.virt = page;

        const API::Result result = VMCtl(SELF, CacheClean, &range);
        if (result != API::Success)
        {
            ERROR("failed to clean data cache at " << (void *) page <<
                  ": result = " << (int) result);
            return IOError;
        }
#endif /* __HOST__ */
    }
    // Clean both pages from the cache directly
    else
    {
        Arch::Cache cache;
        cache.cleanData(page);
    }

    return Success;
}
