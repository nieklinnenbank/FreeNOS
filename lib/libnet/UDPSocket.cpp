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
#include <ByteOrder.h>
#include <Randomizer.h>
#include "Ethernet.h"
#include "UDP.h"
#include "UDPSocket.h"

UDPSocket::UDPSocket(const u32 inode,
                     UDP *udp,
                     const ProcessID pid)
    : NetworkSocket(inode, udp->getMaximumPacketSize(), pid)
    , m_udp(udp)
    , m_port(0)
    , m_queue(udp->getMaximumPacketSize())
{
}

UDPSocket::~UDPSocket()
{
}

const u16 UDPSocket::getPort() const
{
    return m_port;
}

FileSystem::Result UDPSocket::read(IOBuffer & buffer,
                                   Size & size,
                                   const Size offset)
{
    DEBUG("");

    NetworkQueue::Packet *pkt = m_queue.pop();
    if (!pkt)
    {
        return FileSystem::RetryAgain;
    }

    IPV4::Header *ipHdr = (IPV4::Header *)(pkt->data + sizeof(Ethernet::Header));
    UDP::Header *udpHdr = (UDP::Header *)(pkt->data + sizeof(Ethernet::Header) + sizeof(IPV4::Header));
    NetworkClient::SocketInfo info;
    Size payloadSize = pkt->size - sizeof(Ethernet::Header)
                                 - sizeof(IPV4::Header)
                                 - sizeof(UDP::Header);

    // Fill socket info
    info.address = readBe32(&ipHdr->source);
    info.port    = readBe16(&udpHdr->sourcePort);
    buffer.write(&info, sizeof(info));

    // Fill payload
    Size sz = size > payloadSize ? payloadSize : size;
    buffer.write(udpHdr+1, sz, sizeof(info));
    m_queue.release(pkt);
    size = sz + sizeof(info);

    return FileSystem::Success;
}

FileSystem::Result UDPSocket::write(IOBuffer & buffer,
                                    Size & size,
                                    const Size offset)
{
    DEBUG("");

    // Read socket info and action
    NetworkClient::SocketInfo dest;
    buffer.read(&dest, sizeof(dest));

    // Handle the socket operation
    switch (dest.action)
    {
        case NetworkClient::Listen:
        {
            MemoryBlock::copy(&m_info, &dest, sizeof(m_info));

            if (m_info.port == 0)
            {
                Randomizer rand;
                m_info.port = rand.next() % 65535;
            }

            DEBUG("addr =" << m_info.address << " port = " << m_info.port);
            return m_udp->bind(this, m_info.port);
        }

        case NetworkClient::SendSingle:
            return m_udp->sendPacket(&m_info, &dest, buffer, size - sizeof(dest), sizeof(dest));

        case NetworkClient::SendMultiple:
        {
            NetworkClient::PacketInfo packetInfo;
            FileSystemMessage msg;
            IOBuffer io;
            Size packetOffset = 0;

            // Read the first packet info to find the base address for all packets.
            //
            // Note that it is assumed here that all packet buffers
            // originate from the same base address and that each new packet
            // starts after NetworkQueue::PayloadBufferSize bytes.
            buffer.read(&packetInfo, sizeof(packetInfo), sizeof(dest));

            // Prepare dummy filesystem message for the I/O buffer
            msg.from = buffer.getMessage()->from;
            msg.action = FileSystem::WriteFile;
            msg.buffer = (char *)packetInfo.address;
            msg.size = NetworkQueue::MaxPackets * PAGESIZE;
            io.setMessage(&msg);

            // read the array of PacketInfo structs that describe
            // all the packets that need to be transferred
            for (Size i = sizeof(dest); i < size; i += sizeof(NetworkClient::PacketInfo))
            {
                buffer.read(&packetInfo, sizeof(packetInfo), i);
                DEBUG("packet[" << ((i - sizeof(dest)) / sizeof(NetworkClient::PacketInfo)) <<
                      "] size = " << packetInfo.size << " offset = " << packetOffset);

                const FileSystem::Result r = m_udp->sendPacket(&m_info, &dest, io, packetInfo.size, packetOffset);
                if (r != FileSystem::Success)
                {
                    ERROR("failed to send packet: result = " << (int) r);
                    return r;
                }

                packetOffset += NetworkQueue::PayloadBufferSize;
            }

            return FileSystem::Success;
        }

        default:
            return FileSystem::NotSupported;
    }
}

bool UDPSocket::canRead() const
{
    return m_queue.hasData();
}

FileSystem::Result UDPSocket::process(const NetworkQueue::Packet *pkt)
{
    DEBUG("");

    NetworkQueue::Packet *buf = m_queue.get();
    if (!buf)
    {
        ERROR("udp socket queue full");
        return FileSystem::IOError;
    }

    buf->size = pkt->size;
    MemoryBlock::copy(buf->data, pkt->data, pkt->size);
    m_queue.push(buf);

    return FileSystem::Success;
}
