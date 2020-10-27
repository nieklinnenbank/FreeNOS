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
#include "NetworkProtocol.h"
#include "ICMP.h"
#include "ICMPFactory.h"
#include "ICMPSocket.h"
#include "IPV4.h"

ICMP::ICMP(NetworkServer &server,
           NetworkDevice &device,
           NetworkProtocol &parent)
    : NetworkProtocol(server, device, parent)
{
}

ICMP::~ICMP()
{
}

FileSystem::Result ICMP::initialize()
{
    DEBUG("");

    m_factory = new ICMPFactory(this);
    m_server.registerDirectory(this, "/icmp");
    m_server.registerFile(m_factory, "/icmp/factory");

    return FileSystem::Success;
}

FileSystem::Result ICMP::process(const NetworkQueue::Packet *pkt,
                                 const Size offset)
{
    const IPV4::Header *iphdr = (const IPV4::Header *) (pkt->data + offset - sizeof(IPV4::Header));
    const Header *hdr = (const Header *) (pkt->data + offset);
    const IPV4::Address source = readBe32(&iphdr->source);

    DEBUG("source = " << (uint)source << " type = " << hdr->type << " code = " << hdr->code << " id = " << hdr->id);

    switch (hdr->type)
    {
        case EchoRequest:
        {
            DEBUG("request");

            Header reply;
            MemoryBlock::copy(&reply, hdr, sizeof(reply));
            reply.type = EchoReply;
            write16(&reply.checksum, 0);
            write16(&reply.checksum, checksum(&reply));

            return sendPacket(source, &reply);
        }
        case EchoReply:
        {
            DEBUG("reply");

            for (Size i = 0; i < m_sockets.size(); i++)
            {
                ICMPSocket *s = (ICMPSocket *) m_sockets.get(i);
                if (s && s->getAddress() == source)
                    s->setReply(hdr);
            }
            break;
        }
    }

    return FileSystem::Success;
}

ICMPSocket * ICMP::createSocket(String & path)
{
    Size pos = 0;

    DEBUG("");

    // Allocate socket
    ICMPSocket *sock = new ICMPSocket(this);
    if (!sock)
    {
        ERROR("failed to allocate ICMP socket");
        return ZERO;
    }

    // Insert to sockets array
    if (!m_sockets.insert(pos, sock))
    {
        ERROR("failed to insert ICMP socket");
        delete sock;
        return ZERO;
    }
    String filepath;
    filepath << "/icmp/" << pos;

    // Add socket to NetworkServer as a file
    path << m_server.getMountPath() << filepath;
    const FileSystem::Result result = m_server.registerFile(sock, *filepath);
    if (result != FileSystem::Success)
    {
        ERROR("failed to register ICMP socket to NetworkServer: result = " << (int) result);
        m_sockets.remove(pos);
        delete sock;
        return ZERO;
    }

    return sock;
}

FileSystem::Result ICMP::sendPacket(const IPV4::Address ip,
                                    const ICMP::Header *header)
{
    DEBUG("");

    // Get a fresh packet
    NetworkQueue::Packet *pkt = m_parent.getTransmitPacket(&ip, sizeof(ip),
                                                           NetworkProtocol::ICMP, sizeof(Header));
    if (pkt == ZERO)
    {
        return FileSystem::RetryAgain;
    }

    // Fill payload
    MemoryBlock::copy(pkt->data + pkt->size, header, sizeof(ICMP::Header));
    pkt->size += sizeof(ICMP::Header);

    // Transmit the packet
    return m_device.transmit(pkt);
}

const u16 ICMP::checksum(const Header *header)
{
    return IPV4::checksum(header, sizeof(*header));
}
