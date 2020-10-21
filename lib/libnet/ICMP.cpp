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
#include "NetworkProtocol.h"
#include "ICMP.h"
#include "ICMPFactory.h"
#include "ICMPSocket.h"
#include "IPV4.h"

ICMP::ICMP(NetworkServer *server,
           NetworkDevice *device)
    : NetworkProtocol(server, device)
{
    m_ipv4  = ZERO;
}

ICMP::~ICMP()
{
}

void ICMP::setIP(::IPV4 *ip)
{
    m_ipv4 = ip;
}

Error ICMP::initialize()
{
    DEBUG("");
    m_factory = new ICMPFactory(this);
    m_server->registerFile(this, "/icmp");
    m_server->registerFile(m_factory, "/icmp/factory");
    return 0;
}

Error ICMP::process(const NetworkQueue::Packet *pkt,
                    const Size offset)
{
    const IPV4::Header *iphdr = (const IPV4::Header *) (pkt->data + offset - sizeof(IPV4::Header));
    const Header *hdr = (const Header *) (pkt->data + offset);
    const IPV4::Address source = readBe32(&iphdr->source);

    DEBUG("source = " << (uint)source << " type = " << hdr->type << " code = " << hdr->code << " id = " << hdr->id);

    switch (hdr->type)
    {
        case EchoRequest: {
            DEBUG("request");
            Header reply;
            MemoryBlock::copy(&reply, hdr, sizeof(reply));
            reply.type = EchoReply;
            write16(&reply.checksum, 0);
            write16(&reply.checksum, checksum(&reply));
            return sendPacket(source, &reply);
        }
        case EchoReply: {
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
    return 0;
}

ICMPSocket * ICMP::createSocket(String & path)
{
    Size pos = 0;

    DEBUG("");

    ICMPSocket *sock = new ICMPSocket(this);
    if (!sock)
        return ZERO;

    if (!m_sockets.insert(pos, sock))
    {
        delete sock;
        return ZERO;
    }
    String filepath;
    filepath << "/icmp/" << pos;

    path << m_server->getMountPath() << filepath;
    m_server->registerFile(sock, *filepath);
    return sock;
}

Error ICMP::sendPacket(const IPV4::Address ip,
                       const ICMP::Header *header)
{
    DEBUG("");

    NetworkQueue::Packet *pkt;

    // Get a fresh IP packet
    const Error r = m_ipv4->getTransmitPacket(
        &pkt, ip, IPV4::ICMP, sizeof(Header)
    );
    if (r != 0)
    {
        return r;
    }

    // Fill payload
    MemoryBlock::copy(pkt->data + pkt->size, header, sizeof(ICMP::Header));
    pkt->size += sizeof(ICMP::Header);

    // Transmit the packet
    return m_device->transmit(pkt);
}

const u16 ICMP::checksum(const Header *header)
{
    return IPV4::checksum(header, sizeof(*header));
}
