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

#include <FreeNOS/User.h>
#include <Log.h>
#include <String.h>
#include "NetworkQueue.h"

NetworkQueue::NetworkQueue(const Size packetSize,
                           const Size queueSize)
{
    assert(queueSize <= MaxPackets);

    m_payloadRange.virt = ZERO;
    m_payloadRange.phys = ZERO;
    m_payloadRange.size = PayloadBufferSize * queueSize;
    m_payloadRange.access = Memory::User | Memory::Readable | Memory::Writable;

    // Ensure size is page aligned
    if (m_payloadRange.size % PAGESIZE)
    {
        m_payloadRange.size += PAGESIZE - (m_payloadRange.size % PAGESIZE);
    }

    // Allocate memory pages for the payload
    const API::Result result = VMCtl(SELF, MapContiguous, &m_payloadRange);
    if (result != API::Success)
    {
        ERROR("failed to allocate payload buffer: result = " << (int) result);
    }
    else
    {
        // Allocate packet objects
        for (Size i = 0; i < queueSize; i++)
        {
            Packet *packet = new Packet;
            packet->size = 0;
            packet->data = (u8 *) (m_payloadRange.virt + (i * PayloadBufferSize));
            m_free.insertAt(i, packet);
        }
    }
}

NetworkQueue::~NetworkQueue()
{
    for (Size i = 0; i < m_free.size(); i++)
    {
        Packet *p = m_free.get(i);
        if (p)
        {
            delete p;
        }
    }

    VMCtl(SELF, Release, &m_payloadRange);
}

NetworkQueue::Packet * NetworkQueue::get()
{
    for (Size i = 0; i < m_free.size(); i++)
    {
        Packet *p = m_free.get(i);
        if (p)
        {
            p->size = 0;
            m_free.remove(i);
            return p;
        }
    }
    return ZERO;
}

void NetworkQueue::release(NetworkQueue::Packet *packet)
{
    packet->size = 0;
    m_free.insert(packet);
}

void NetworkQueue::push(NetworkQueue::Packet *packet)
{
    m_data.insert(packet);
}

NetworkQueue::Packet * NetworkQueue::pop()
{
    for (Size i = 0; i < m_data.size(); i++)
    {
        Packet *p = m_data.get(i);
        if (p)
        {
            m_data.remove(i);
            return p;
        }
    }
    return ZERO;
}

bool NetworkQueue::hasData() const
{
    return m_data.count() > 0;
}

Log & operator << (Log &log, const NetworkQueue::Packet & pkt)
{
    String s;

    for (Size i = 0; i < pkt.size; i++)
    {
        if ((i % 16) == 0)
            s << "\r\n";

        const uint val = pkt.data[i];
        s << Number::Hex << val;

        if (val >= 0x10)
            s << " ";
        else
            s << "  ";
    }
    s << "\r\n";

    log.append(*s);
    return log;
}
