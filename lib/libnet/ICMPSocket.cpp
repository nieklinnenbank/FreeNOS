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

#include <MemoryBlock.h>
#include "Ethernet.h"
#include "IPV4.h"
#include "ICMP.h"
#include "ICMPSocket.h"
#include "NetworkClient.h"

ICMPSocket::ICMPSocket(const u32 inode,
                       ICMP *icmp,
                       const ProcessID pid)
    : NetworkSocket(inode, icmp->getMaximumPacketSize(), pid)
{
    m_icmp = icmp;
    m_gotReply = false;
    MemoryBlock::set(&m_info, 0, sizeof(m_info));
}

ICMPSocket::~ICMPSocket()
{
}

const IPV4::Address ICMPSocket::getAddress() const
{
    return m_info.address;
}

FileSystem::Result ICMPSocket::read(IOBuffer & buffer,
                                    Size & size,
                                    const Size offset)
{
    DEBUG("");

    if (!m_gotReply)
    {
        return FileSystem::RetryAgain;
    }

    m_gotReply = false;
    buffer.write(&m_reply, sizeof(m_reply));
    size = sizeof(m_reply);
    return FileSystem::Success;
}

FileSystem::Result ICMPSocket::write(IOBuffer & buffer,
                                     Size & size,
                                     const Size offset)
{
    DEBUG("");

    // Receive socket information first?
    if (!m_info.address)
    {
        buffer.read(&m_info, sizeof(m_info));
        return FileSystem::Success;
    }
    else
    {
        ICMP::Header header;
        buffer.read(&header, sizeof(header));

        return m_icmp->sendPacket(m_info.address, &header, ZERO, 0);
    }
}

FileSystem::Result ICMPSocket::process(const NetworkQueue::Packet *pkt)
{
    DEBUG("");

    return FileSystem::Success;
}

void ICMPSocket::setReply(const ICMP::Header *header)
{
    DEBUG("");

    if (!m_gotReply)
    {
        MemoryBlock::copy(&m_reply, header, sizeof(ICMP::Header));
        m_gotReply = true;
    }
}
