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
#include "NetworkServer.h"
#include "NetworkDevice.h"
#include "IPV4.h"
#include "IPV4Address.h"
#include "UDP.h"
#include "ICMP.h"

IPV4::IPV4(NetworkServer &server,
           NetworkDevice &device,
           NetworkProtocol &parent)
    : NetworkProtocol(server, device, parent)
{
    m_address = 0;
    m_icmp = 0;
    m_udp = 0;
    m_id = 1;
}

IPV4::~IPV4()
{
}

FileSystem::Result IPV4::initialize()
{
    DEBUG("");

    m_server.registerDirectory(this, "/ipv4");
    m_server.registerFile(new IPV4Address(m_server.getNextInode(), this), "/ipv4/address");

    return FileSystem::Success;
}

void IPV4::setICMP(::ICMP *icmp)
{
    m_icmp = icmp;
}

void IPV4::setARP(::ARP *arp)
{
    m_arp = arp;
}

void IPV4::setUDP(::UDP *udp)
{
    m_udp = udp;
}

FileSystem::Result IPV4::getAddress(IPV4::Address *address)
{
    *address = m_address;
    return FileSystem::Success;
}

FileSystem::Result IPV4::setAddress(const IPV4::Address *address)
{
    m_address = *address;
    return FileSystem::Success;
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
    {
        return ZERO;
    }

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

IPV4::Protocol IPV4::getProtocolByIdentifier(const NetworkProtocol::Identifier id) const
{
    switch (id)
    {
        case NetworkProtocol::ICMP:
            return IPV4::ICMP;
        case NetworkProtocol::UDP:
            return IPV4::UDP;
        default:
            return IPV4::TCP;
    }
}

FileSystem::Result IPV4::getTransmitPacket(NetworkQueue::Packet **pkt,
                                           const void *address,
                                           const Size addressSize,
                                           const NetworkProtocol::Identifier protocol,
                                           const Size payloadSize)
{
    Ethernet::Address ethAddr;

    // Find the ethernet address using ARP first
    FileSystem::Result result = m_arp->lookupAddress((const IPV4::Address *)address, &ethAddr);
    if (result != FileSystem::Success)
    {
        if (result != FileSystem::RetryAgain)
        {
            ERROR("failed to perform ARP lookup: result = " << (int) result);
        }
        return result;
    }

    // Get a fresh ethernet packet
    result = m_parent.getTransmitPacket(pkt, &ethAddr, sizeof(ethAddr),
                                        NetworkProtocol::IPV4, payloadSize);
    if (result != FileSystem::Success)
    {
        if (result != FileSystem::RetryAgain)
        {
            ERROR("failed to get transmit packet: result = " << (int) result);
        }
        return result;
    }

    // Fill IP header
    Header *hdr = (Header *) ((*pkt)->data + (*pkt)->size);
    hdr->versionIHL     = (sizeof(Header) / sizeof(u32)) | (4 << 4);
    hdr->typeOfService  = 0;
    hdr->timeToLive     = 64;
    hdr->protocol       = getProtocolByIdentifier(protocol);
    writeBe16(&hdr->length, payloadSize + sizeof(Header));
    writeBe16(&hdr->identification, m_id);
    writeBe16(&hdr->fragmentOffset, 0x4000); // dont fragment flag
    writeBe32(&hdr->source, m_address);
    writeBe32(&hdr->destination, *(u32 *)address);
    hdr->checksum       = 0;
    hdr->checksum       = checksum(hdr, sizeof(Header));
    (*pkt)->size += sizeof(Header);
    m_id++;

    // Success
    return FileSystem::Success;
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

FileSystem::Result IPV4::process(const NetworkQueue::Packet *pkt,
                                 const Size offset)
{
    DEBUG("");

    const Header *hdr = (const Header *) (pkt->data + offset);
    const u32 destination = readBe32(&hdr->destination);

    if (destination != m_address && destination != 0xffffffff && m_address != 0)
    {
        DEBUG("dropped packet for " << *IPV4::toString(destination));
        return FileSystem::NotFound;
    }

    switch (hdr->protocol)
    {
        case ICMP:
            return m_icmp->process(pkt, offset + sizeof(Header));

        case UDP:
            return m_udp->process(pkt, offset + sizeof(Header));

        default:
            break;
    }

    return FileSystem::InvalidArgument;
}
