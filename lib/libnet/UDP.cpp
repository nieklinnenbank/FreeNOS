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
#include "NetworkServer.h"
#include "NetworkDevice.h"
#include "UDP.h"
#include "UDPSocket.h"
#include "UDPFactory.h"

UDP::UDP(NetworkServer &server,
         NetworkDevice &device,
         NetworkProtocol &parent)
    : NetworkProtocol(server, device, parent)
{
}

UDP::~UDP()
{
}

FileSystem::Result UDP::initialize()
{
    DEBUG("");

    m_factory = new UDPFactory(m_server.getNextInode(), this);
    m_server.registerDirectory(this, "/udp");
    m_server.registerFile(m_factory, "/udp/factory");

    return FileSystem::Success;
}

UDPSocket * UDP::createSocket(String & path,
                              const ProcessID pid)
{
    Size pos = 0;

    DEBUG("");

    UDPSocket *sock = new UDPSocket(m_server.getNextInode(), this, pid);
    if (!sock)
    {
        ERROR("failed to allocate UDP socket");
        return ZERO;
    }

    if (!m_sockets.insert(pos, sock))
    {
        ERROR("failed to insert UDP socket");
        delete sock;
        return ZERO;
    }
    String filepath;
    filepath << "/udp/" << pos;

    path << m_server.getMountPath() << filepath;
    const FileSystem::Result result = m_server.registerFile(sock, *filepath);
    if (result != FileSystem::Success)
    {
        ERROR("failed to register UDP socket: result = " << (int) result);
        delete sock;
        m_sockets.remove(pos);
    }

    return sock;
}

void UDP::unregisterSockets(const ProcessID pid)
{
    DEBUG("pid = " << pid);

    for (HashIterator<u16, UDPSocket *> it(m_ports); it.hasCurrent();)
    {
        UDPSocket *sock = it.current();
        if (sock->getProcessID() == pid)
        {
            it.remove();
        }
        else
        {
            it++;
        }
    }

    for (Size i = 0; i < MaxUdpSockets; i++)
    {
        UDPSocket *sock = m_sockets[i];
        if (sock != ZERO && sock->getProcessID() == pid)
        {
            m_sockets.remove(i);
            String path;
            path << "/udp/" << i;
            const FileSystem::Result result = m_server.unregisterFile(*path);
            if (result != FileSystem::Success)
            {
                ERROR("failed to unregister UDPSocket at " << *path <<
                      " for PID " << pid << ": result = " << (int) result);
            }
        }
    }
}

FileSystem::Result UDP::process(const NetworkQueue::Packet *pkt,
                                const Size offset)
{
    const Header *hdr = (const Header *)(pkt->data + sizeof(Ethernet::Header) + sizeof(IPV4::Header));
    const u16 port = be16_to_cpu(hdr->destPort);

    DEBUG("port = " << port);

    // Process the packet if we have a socket on that port
    UDPSocket **sock = (UDPSocket **) m_ports.get(port);
    if (!sock)
    {
        DEBUG("dropped");
        return FileSystem::NotFound;
    }

    (*sock)->process(pkt);
    return FileSystem::Success;
}

FileSystem::Result UDP::sendPacket(const NetworkClient::SocketInfo *src,
                                   const NetworkClient::SocketInfo *dest,
                                   IOBuffer & buffer,
                                   const Size size,
                                   const Size offset)
{
    NetworkQueue::Packet *pkt;
    Header *hdr;

    DEBUG("address = " << *IPV4::toString(dest->address) <<
          " port = " << dest->port << " size = " << size);

    // Get a fresh packet
    const FileSystem::Result result = m_parent.getTransmitPacket(&pkt, &dest->address, sizeof(dest->address),
                                                                 NetworkProtocol::UDP, sizeof(Header) + size);
    if (result != FileSystem::Success)
    {
        if (result != FileSystem::RetryAgain)
        {
            ERROR("failed to get transmit packet: result = " << (int) result);
        }
        return result;
    }

    // Fill UDP header
    hdr = (Header *) (pkt->data + pkt->size);
    writeBe16(&hdr->sourcePort, src->port);
    writeBe16(&hdr->destPort, dest->port);
    writeBe16(&hdr->length, size + sizeof(Header));
    writeBe16(&hdr->checksum, 0);

    // Insert payload. The payload is read from the given offset in the IOBuffer.
    // Note that the payload must not overwrite past the packet buffer
    const Size maximum = getMaximumPacketSize();
    const Size needed = pkt->size + size;

    buffer.read(pkt->data + pkt->size + sizeof(Header),
                needed > maximum ? maximum : needed, offset);

    // Calculate final checksum
    write16(&hdr->checksum, checksum((IPV4::Header *)(pkt->data + pkt->size - sizeof(IPV4::Header)),
                                      hdr, size));
    DEBUG("checksum = " << (uint) hdr->checksum);

    // Increment packet size
    pkt->size += sizeof(Header) + size;

    // Transmit now
    return m_device.transmit(pkt);
}

FileSystem::Result UDP::bind(UDPSocket *sock,
                             const u16 port)
{
    DEBUG("port = " << port);

    if (!port)
    {
        return FileSystem::InvalidArgument;
    }

    m_ports.insert(port, sock);
    return FileSystem::Success;
}

const ulong UDP::calculateSum(const u16 *ptr,
                              const Size bytes)
{
    ulong sum = 0;
    Size remain = bytes;

    for (;remain > 0; remain -= sizeof(u16), ptr++)
    {
        if (remain == sizeof(u8))
        {
            sum += read8(ptr);
            break;
        }
        else
        {
            sum += read16(ptr);
        }
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
    phr.source = read32(&ip->source);
    phr.destination = read32(&ip->destination);
    writeBe16(&phr.length, sizeof(Header) + datalen);
    DEBUG("ip src = " << *IPV4::toString(phr.source) <<
          " dst = " << *IPV4::toString(phr.destination));

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
