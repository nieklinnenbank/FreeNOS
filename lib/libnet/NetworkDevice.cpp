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

NetworkDevice::NetworkDevice(NetworkServer *server)
    : Device(CharacterDeviceFile),
      m_receive(1500),
      m_transmit(1500)
{
    m_maximumPacketSize = 1500;
    m_server = server;
    m_eth = 0;
    m_arp = 0;
    m_ipv4 = 0;
    m_udp = 0;
}

NetworkDevice::~NetworkDevice()
{
}

Error NetworkDevice::initialize()
{
    Error r = Device::initialize();
    if (r != ESUCCESS)
    {
        ERROR("failed to initialize Device");
        return r;
    }

    // Setup protocol stack
    m_eth  = new Ethernet(m_server, this);
    m_arp  = new ARP(m_server, this);
    m_ipv4 = new IPV4(m_server, this);
    m_icmp = new ICMP(m_server, this);
    m_udp  = new UDP(m_server, this);

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
    m_arp->setEthernet(m_eth);
    m_ipv4->setICMP(m_icmp);
    m_ipv4->setARP(m_arp);
    m_ipv4->setEthernet(m_eth);
    m_ipv4->setUDP(m_udp);
    m_icmp->setIP(m_ipv4);
    m_udp->setIP(m_ipv4);
    return r;
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

Error NetworkDevice::process(NetworkQueue::Packet *pkt, Size offset)
{
    DEBUG("");

    // Let the protocols process the packet
    m_eth->process(pkt, offset);
    return ESUCCESS;
}
