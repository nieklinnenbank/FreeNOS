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

#include "NetworkDevice.h"

NetworkDevice::NetworkDevice(const u32 inode,
                             NetworkServer &server)
    : Device(inode, FileSystem::CharacterDeviceFile)
    , m_maximumPacketSize(1500)
    , m_receive(m_maximumPacketSize)
    , m_transmit(m_maximumPacketSize)
    , m_server(server)

{
    // Allocate protocols. Note that the FileSystemServer will take
    // ownership of these objects via the FileCache hierarchy
    m_eth = new Ethernet(m_server, *this);
    m_arp = new ARP(m_server, *this, *m_eth);
    m_ipv4 = new IPV4(m_server, *this, *m_eth);
    m_icmp = new ICMP(m_server, *this, *m_ipv4);
    m_udp = new UDP(m_server, *this, *m_ipv4);
}

NetworkDevice::~NetworkDevice()
{
}

FileSystem::Result NetworkDevice::initialize()
{
    const FileSystem::Result result = Device::initialize();
    if (result != FileSystem::Success)
    {
        ERROR("failed to initialize Device: result = " << (int) result);
        return result;
    }

    // Initialize protocols
    m_eth->initialize();
    m_arp->initialize();
    m_ipv4->initialize();
    m_icmp->initialize();
    m_udp->initialize();

    // Connect objects
    m_eth->setIP(m_ipv4);
    m_eth->setARP(m_arp);
    m_arp->setIP(m_ipv4);
    m_ipv4->setICMP(m_icmp);
    m_ipv4->setARP(m_arp);
    m_ipv4->setUDP(m_udp);

    return FileSystem::Success;
}

const Size NetworkDevice::getMaximumPacketSize() const
{
    return m_maximumPacketSize;
}

NetworkQueue * NetworkDevice::getReceiveQueue()
{
    return &m_receive;
}

NetworkQueue * NetworkDevice::getTransmitQueue()
{
    return &m_transmit;
}

void NetworkDevice::unregisterSockets(const ProcessID pid)
{
    DEBUG("pid = " << pid);

    m_udp->unregisterSockets(pid);
    m_icmp->unregisterSockets(pid);
}

FileSystem::Result NetworkDevice::process(const NetworkQueue::Packet *pkt,
                                          const Size offset)
{
    DEBUG("");

    // Let the protocols process the packet
    return m_eth->process(pkt, offset);
}

FileSystem::Result NetworkDevice::startDMA()
{
    DEBUG("");

    return FileSystem::Success;
}
