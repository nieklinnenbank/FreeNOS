/*
 * Copyright (C) 2021 Niek Linnenbank
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

#include <MemoryBlock.h>
#include <NetworkClient.h>
#include <NetworkSocket.h>
#include <String.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include "NetSend.h"

NetSend::NetSend(int argc, char **argv)
    : POSIXApplication(argc, argv)
{
    parser().setDescription("send network packets");
    parser().registerPositional("DEVICE", "device name of network adapter");
    parser().registerPositional("HOST", "host address to send to");
    parser().registerPositional("PORT", "UDP port to use");
    parser().registerPositional("COUNT", "number of packets to send");
}

NetSend::~NetSend()
{
}

NetSend::Result NetSend::initialize()
{
    const char *dev  = arguments().get("DEVICE");
    const u16 port = String(arguments().get("PORT")).toLong();

    DEBUG("");

    // Create a network client
    m_client = new NetworkClient(dev);

    // Initialize networking client
    NetworkClient::Result result = m_client->initialize();
    if (result != NetworkClient::Success)
    {
        ERROR("failed to initialize network client for device "
               << dev << ": result = " << (int) result);
        return IOError;
    }

    // Create an UDP socket
    result = m_client->createSocket(NetworkClient::UDP, &m_socket);
    if (result != NetworkClient::Success)
    {
        ERROR("failed to create UDP socket on device " << dev <<
              ": result = " << (int) result);
        return IOError;
    }

    // Bind to a local port.
    result = m_client->bindSocket(m_socket, 0, port);
    if (result != NetworkClient::Success)
    {
        ERROR("failed to bind socket to UDP port " << port <<
              " on device " << dev << ": result = " << (int) result);
        return IOError;
    }

    return Success;
}

NetSend::Result NetSend::exec()
{
    const IPV4::Address host = IPV4::toAddress(arguments().get("HOST"));
    const u16 port = String(arguments().get("PORT")).toLong();
    const Size count = String(arguments().get("COUNT")).toLong();

    DEBUG(" host = " << *IPV4::toString(host) << "port = " << port << " count = " << count);

    // Prepare the socket address to send to
    struct sockaddr addr;
    addr.addr = host;
    addr.port = port;

    // Prepare I/O vector with generated packets for sending
    static u8 pkts[NetworkQueue::MaxPackets][NetworkQueue::PayloadBufferSize];
    static struct iovec vec[QueueSize];

    for (Size i = 0; i < QueueSize; i++)
    {
        MemoryBlock::set(pkts[i], i, PacketSize);

        vec[i].iov_base = pkts[i];
        vec[i].iov_len = PacketSize;
    }

    // Keep sending packets until we reach the number to send
    for (Size i = 0; i < count;)
    {
        const Size num = count - i >= QueueSize ?
                         QueueSize : count - i;

        const Result r = udpSendMultiple(vec, num, addr);
        if (r != Success)
        {
            ERROR("failed to send multiple UDP packets: result = " << (int) r);
            return r;
        }

        i += num;
    }

    return Success;
}

NetSend::Result NetSend::udpSendMultiple(const struct iovec *vec,
                                         const Size count,
                                         const struct sockaddr & addr) const
{
    struct msghdr msg;

    DEBUG("host = " << *IPV4::toString(addr.addr) << " port = " << addr.port << " count = " << count);

    // Prepare the message header
    msg.msg_name = (void *) &addr;
    msg.msg_namelen = sizeof(addr);
    msg.msg_iov = (struct iovec *) vec;
    msg.msg_iovlen = count;

    // Send the packet
    int result = ::sendmsg(m_socket, &msg, 0);
    if (result <= 0)
    {
        ERROR("failed to send multiple UDP datagrams: " << strerror(errno));
        return IOError;
    }

    return Success;
}
