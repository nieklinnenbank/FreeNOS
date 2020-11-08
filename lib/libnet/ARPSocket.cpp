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

#include <FreeNOS/API/ProcessID.h>
#include "Ethernet.h"
#include "IPV4.h"
#include "ARP.h"
#include "ARPSocket.h"

ARPSocket::ARPSocket(const u32 inode,
                     ARP *arp)
    : NetworkSocket(inode, arp->getMaximumPacketSize(), ANY)
{
    m_arp = arp;
    m_ipAddr = 0;
    MemoryBlock::set(&m_ethAddr, 0, sizeof(Ethernet::Address));
}

ARPSocket::~ARPSocket()
{
}

FileSystem::Result ARPSocket::read(IOBuffer & buffer,
                                   Size & size,
                                   const Size offset)
{
    DEBUG("");

    if (size != sizeof(Ethernet::Address))
    {
        return FileSystem::InvalidArgument;
    }

    if (offset >= sizeof(Ethernet::Address))
    {
        size = 0;
        return FileSystem::Success;
    }

    const FileSystem::Result result = m_arp->lookupAddress(&m_ipAddr, &m_ethAddr);
    if (result != FileSystem::Success)
    {
        return result;
    }

    buffer.write(&m_ethAddr, sizeof(Ethernet::Address));
    size = sizeof(Ethernet::Address);
    return FileSystem::Success;
}

FileSystem::Result ARPSocket::write(IOBuffer & buffer,
                                    Size & size,
                                    const Size offset)
{
    DEBUG("");

    // Save the request address
    buffer.read(&m_ipAddr, sizeof(IPV4::Address));

    // Send request
    const FileSystem::Result result = m_arp->lookupAddress(&m_ipAddr, &m_ethAddr);
    if (result != FileSystem::Success)
    {
        return result;
    }

    size = sizeof(IPV4::Address);
    return FileSystem::Success;
}

FileSystem::Result ARPSocket::process(const NetworkQueue::Packet *pkt)
{
    DEBUG("");

    return FileSystem::Success;
}
