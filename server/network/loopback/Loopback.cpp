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

#include "Loopback.h"

Loopback::Loopback(const u32 inode,
                   NetworkServer &server)
    : NetworkDevice(inode, server)
{
    DEBUG("");

    m_address.addr[0] = 0x11;
    m_address.addr[1] = 0x22;
    m_address.addr[2] = 0x33;
    m_address.addr[3] = 0x44;
    m_address.addr[4] = 0x55;
    m_address.addr[5] = 0x66;
}

Loopback::~Loopback()
{
    DEBUG("");
}

FileSystem::Result Loopback::initialize()
{
    DEBUG("");

    const FileSystem::Result result = NetworkDevice::initialize();
    if (result != FileSystem::Success)
    {
        ERROR("failed to initialize NetworkDevice: result = " << (int) result);
        return FileSystem::IOError;
    }

    IPV4::Address addr = IPV4::toAddress("127.0.0.1");
    m_ipv4->setAddress(&addr);

    return FileSystem::Success;
}

FileSystem::Result Loopback::getAddress(Ethernet::Address *address)
{
    DEBUG("");

    MemoryBlock::copy(address, &m_address, sizeof(Ethernet::Address));
    return FileSystem::Success;
}

FileSystem::Result Loopback::setAddress(const Ethernet::Address *address)
{
    DEBUG("");

    MemoryBlock::copy(&m_address, address, sizeof(Ethernet::Address));
    return FileSystem::Success;
}

FileSystem::Result Loopback::transmit(NetworkQueue::Packet *pkt)
{
    DEBUG("size = " << pkt->size);

    // Process the packet by protocols as input (loopback)
    const FileSystem::Result result = process(pkt);

    // Release packet buffer
    m_transmit.release(pkt);

    // Done
    return result;
}
