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

#include <ByteOrder.h>
#include <errno.h>
#include "NetworkServer.h"
#include "NetworkDevice.h"
#include "UDP.h"
#include "UDPSocket.h"
#include "UDPFactory.h"

UDP::UDP(NetworkServer *server,
         NetworkDevice *device)
    : NetworkProtocol(server, device)
{
    m_ipv4 = 0;
}

UDP::~UDP()
{
}

void UDP::setIP(::IPV4 *ip)
{
    m_ipv4 = ip;
}

Error UDP::initialize()
{
    DEBUG("");
    m_factory = new UDPFactory(this);
    m_server->registerFile(this, "/udp");
    m_server->registerFile(m_factory, "/udp/factory");
    return 0;
}

UDPSocket * UDP::createSocket(String & path)
{
    Size pos = 0;

    DEBUG("");

    UDPSocket *sock = new UDPSocket(this);
    if (!sock)
        return ZERO;

    if (!m_sockets.insert(sock))
    {
        delete sock;
        return ZERO;
    }
    String filepath;
    filepath << "/udp/" << pos;

    path << m_server->getMountPath() << filepath;
    m_server->registerFile(sock, *filepath);
    return sock;
}

Error UDP::process(NetworkQueue::Packet *pkt, Size offset)
{
    Header *hdr = (Header *)(pkt->data + sizeof(Ethernet::Header) + sizeof(IPV4::Header));
    u16 port = be16_to_cpu(hdr->destPort);

    DEBUG("port = " << port);

    // Process the packet if we have a socket on that port
    UDPSocket **sock = (UDPSocket **) m_ports.get(port);
    if (!sock)
    {
        DEBUG("dropped");
        return EINVAL;
    }
    (*sock)->process(pkt);
    return 0;
}

Error UDP::sendPacket(NetworkClient::SocketInfo *src, IOBuffer & buffer, Size size)
{
    NetworkClient::SocketInfo dest;
    NetworkQueue::Packet *pkt;
    Header *hdr;
    Error r;

    DEBUG("");

    // Read destination
    buffer.read(&dest, sizeof(dest));
    DEBUG("send payload to: " << dest.address << " port: " << dest.port << " size: " << size);

    // Get a fresh IP packet
    r = m_ipv4->getTransmitPacket(
        &pkt, dest.address, IPV4::UDP, sizeof(Header) + size - sizeof(dest)
    );
    if (r != 0)
        return r;

    // Fill UDP header
    hdr = (Header *) (pkt->data + pkt->size);
    writeBe16(&hdr->sourcePort, src->port);
    writeBe16(&hdr->destPort, dest.port);
    writeBe16(&hdr->length, size - sizeof(dest) + sizeof(Header));
    writeBe16(&hdr->checksum, 0);

    // Insert payload. The payload is just after the 'dest' struct in the IOBuffer.
    buffer.read(pkt->data + pkt->size + sizeof(Header), size - sizeof(dest), sizeof(dest));

    // Calculate final checksum
    writeBe16(&hdr->checksum, checksum((IPV4::Header *)(pkt->data + pkt->size - sizeof(IPV4::Header)),
                                        hdr, size - sizeof(dest)));
    DEBUG("checksum = " << (uint) hdr->checksum);

    // Increment packet size
    pkt->size += sizeof(Header) + size - sizeof(dest);

    // Transmit now
    return m_device->transmit(pkt);
}

Error UDP::bind(UDPSocket *sock, u16 port)
{
    DEBUG("port = " << port);

    if (!port)
        return EINVAL;

    m_ports.insert(port, sock);
    return 0;
}

const ulong UDP::calculateSum(const u16 *ptr, Size bytes)
{
    ulong sum = 0;

    for (;bytes > 0; bytes -= 2, ptr++)
    {
        if (bytes == 1) {
            sum += read8(ptr);
            break;
        }
        else
            sum += read16(ptr);
    }

    return sum;
}

const u16 UDP::checksum(const IPV4::Header *ip,
                        const UDP::Header *udp,
                        const Size datalen)
{
    IPV4::PseudoHeader phr;
    ulong sum = 0;

    // Setup a pseudo header
    phr.reserved = 0;
    phr.protocol = IPV4::UDP;
    write32(&phr.source, ip->source);
    write32(&phr.destination, ip->destination);
    writeBe16(&phr.length, sizeof(Header) + datalen);
    DEBUG("ip src = " << phr.source << " dst = " << phr.destination);

    // Sum the pseudo header
    sum += calculateSum((u16 *) &phr, sizeof(phr));
    sum += calculateSum((u16 *) udp, sizeof(*udp) + datalen);

    // Keep only last 16 bits and add carry bits
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    // Take one's complement
    sum = ~sum;
    return (u16) sum;
}
