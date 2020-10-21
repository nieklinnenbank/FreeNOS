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

#include <FreeNOS/System.h>
#include <String.h>
#include <List.h>
#include <ListIterator.h>
#include <ByteOrder.h>
#include <errno.h>
#include "NetworkServer.h"
#include "NetworkDevice.h"
#include "IPV4.h"
#include "IPV4Address.h"
#include "UDP.h"
#include "ICMP.h"

IPV4::IPV4(NetworkServer *server,
           NetworkDevice *device)
    : NetworkProtocol(server, device)
{
    m_address = 0;
    m_icmp = 0;
    m_udp = 0;
    m_ether = 0;
    m_id = 1;
}

IPV4::~IPV4()
{
}

Error IPV4::initialize()
{
    DEBUG("");
    m_server->registerFile(this, "/ipv4");
    m_server->registerFile(new IPV4Address(this), "/ipv4/address");
    return 0;
}

void IPV4::setICMP(::ICMP *icmp)
{
    m_icmp = icmp;
}

void IPV4::setEthernet(::Ethernet *ether)
{
    m_ether = ether;
}

void IPV4::setARP(::ARP *arp)
{
    m_arp = arp;
}

void IPV4::setUDP(::UDP *udp)
{
    m_udp = udp;
}

Error IPV4::getAddress(IPV4::Address *address)
{
    *address = m_address;
    return 0;
}

Error IPV4::setAddress(const IPV4::Address *address)
{
    m_address = *address;
    return 0;
}

const String IPV4::toString(const Address address)
{
    String s;

    s << ((address >> 24) & 0xff) << "."
      << ((address >> 16) & 0xff) << "."
      << ((address >> 8)  & 0xff) << "."
      << ((address & 0xff));

    return s;
}

const IPV4::Address IPV4::toAddress(const char *address)
{
    const String input = address;
    const List<String> lst = input.split('.');
    Size shift = 24;
    IPV4::Address addr = 0;

    // The address must be 4 bytes
    if (lst.count() != 4)
        return ZERO;

    // Extract bytes in dot format (xxx.xxx.xxx.xxx)
    for (ListIterator<String> i(lst); i.hasCurrent(); i++)
    {
        String & s = i.current();
        u8 byte = s.toLong();
        addr |= (byte << shift);
        shift -= 8;
    }

    // Done
    return addr;
}

Error IPV4::getTransmitPacket(NetworkQueue::Packet **pkt,
                              const IPV4::Address destination,
                              const IPV4::Protocol type,
                              const Size size)
{
    Ethernet::Address ethAddr;
    Error r;

    // Find the ethernet address using ARP first
    if ((r = m_arp->lookupAddress(&destination, &ethAddr)) != 0)
        return r;

    // Get a fresh ethernet packet
    *pkt = m_ether->getTransmitPacket(
        &ethAddr,
        Ethernet::IPV4
    );
    // Fill IP header
    Header *hdr = (Header *) ((*pkt)->data + (*pkt)->size);
    hdr->versionIHL     = (sizeof(Header) / sizeof(u32)) | (4 << 4);
    hdr->typeOfService  = 0;
    hdr->timeToLive     = 64;
    hdr->protocol       = type;
    writeBe16(&hdr->length, size + sizeof(Header));
    writeBe16(&hdr->identification, m_id);
    writeBe16(&hdr->fragmentOffset, 0x4000); // dont fragment flag
    writeBe32(&hdr->source, m_address);
    writeBe32(&hdr->destination, destination);
    hdr->checksum       = 0;
    hdr->checksum       = checksum(hdr, sizeof(Header));
    (*pkt)->size += sizeof(Header);
    m_id++;

    // Success
    return 0;
}

const u16 IPV4::checksum(const void *buffer, const Size length)
{
    const u16 *ptr = (const u16 *) buffer;
    Size len = length;
    uint sum = 0;

    // Calculate sum of the buffer
    while (len > 1)
    {
        sum += read16(ptr);
        ptr++;
        len -= 2;
    }

    // Add left-over byte, if any
    if (len > 0)
        sum += (readBe16(ptr) << 8);

    // Enforce 16-bit checksum
    while (sum >> 16)
        sum = (sum >> 16) + (sum & 0xFFFF);

    // Convert to one's complement
    return (~sum);
}

Error IPV4::process(const NetworkQueue::Packet *pkt,
                    const Size offset)
{
    DEBUG("");

    const Header *hdr = (const Header *) (pkt->data + offset);
    const u32 destination = readBe32(&hdr->destination);

    if (destination != m_address)
    {
        DEBUG("dropped packet");
        return ERANGE;
    }

    switch (hdr->protocol)
    {
        case ICMP:
            m_icmp->process(pkt, offset + sizeof(Header));
            break;

        case UDP:
            m_udp->process(pkt, offset + sizeof(Header));
            break;

        default:
            break;
    }

    return EINVAL;
}
