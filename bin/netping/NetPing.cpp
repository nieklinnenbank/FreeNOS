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

#include <MemoryBlock.h>
#include <NetworkClient.h>
#include <NetworkSocket.h>
#include <IPV4.h>
#include <ICMP.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "NetPing.h"

NetPing::NetPing(int argc, char **argv)
    : POSIXApplication(argc, argv)
{
    parser().setDescription("send network pings");
    parser().registerPositional("DEVICE", "device name of network adapter");
    parser().registerPositional("HOST", "host address to ping");
    parser().registerFlag('a', "arp", "send ARP pings");
    parser().registerFlag('i', "icmp", "send ICMP pings");
}

NetPing::~NetPing()
{
}

NetPing::Result NetPing::exec()
{
    DEBUG("");

    const char *dev  = arguments().get("DEVICE");
    const char *host = arguments().get("HOST");
    const char *icmp = arguments().get("icmp");

    DEBUG("sending on device: " << dev);

    if (icmp)
    {
        DEBUG("sending ICMP packets");
        return icmpPing(dev, host);
    }
    else
    {
        DEBUG("sending ARP packets");
        return arpPing(dev, host);
    }
}

NetPing::Result NetPing::arpPing(const char *dev,
                                 const char *host)
{
    NetworkClient client(dev);
    NetworkClient::Result result;
    Ethernet::Address ethAddr;
    IPV4::Address ipAddr;
    int sock;

    DEBUG("");

    // Initialize networking client
    result = client.initialize();
    if (result != NetworkClient::Success)
    {
        ERROR("failed to initialize network client for device: " << dev <<
              ", result = " << (int) result);
        return IOError;
    }

    // Create an ARP socket
    result = client.createSocket(NetworkClient::ARP, &sock);
    if (result != NetworkClient::Success)
    {
        ERROR("failed to create ARP socket: result = " << (int) result);
        return IOError;
    }

    // Convert to IPV4 address
    if (!(ipAddr = IPV4::toAddress(host)))
    {
        ERROR("failed to convert to IPV4 address: " << host);
        return IOError;
    }
    printf("Sending ARP request to %s\r\n", host);

    // Send ARP request
    if (::write(sock, &ipAddr, sizeof(ipAddr)) < 0)
    {
        ERROR("failed to send ARP request: " << strerror(errno));
        return IOError;
    }

    // Receive ARP reply, if any
    if (::read(sock, &ethAddr, sizeof(ethAddr)) < 0)
    {
        ERROR("failed to receive ARP response: " << strerror(errno));
        return IOError;
    }

    // Print the MAC address received
    printf("Received ARP response for: ");

    for (Size i = 0; i < sizeof(Ethernet::Address); i++)
        printf("%x:", ethAddr.addr[i]);
    printf("\r\n");

    // Finished
    client.close(sock);
    return Success;
}

NetPing::Result NetPing::icmpPing(const char *dev, const char *host)
{
    NetworkClient client(dev);
    NetworkClient::Result result;
    int sock;

    DEBUG("");

    // Initialize networking client
    result = client.initialize();
    if (result != NetworkClient::Success)
    {
        ERROR("failed to initialize network client for device: " << dev <<
              ", result = " << (int) result);
        return IOError;
    }

    // Create an ICMP socket
    result = client.createSocket(NetworkClient::ICMP, &sock);
    if (result != NetworkClient::Success)
    {
        ERROR("failed to create ICMP socket: result = " << (int)result);
        return IOError;
    }

    // Connect socket to the given host
    result = client.connectSocket(sock, IPV4::toAddress(host));
    if (result != NetworkClient::Success)
    {
        ERROR("failed to connect ICMP socket: result = " << (int) result);
        return IOError;
    }

    // Send an echo request
    ICMP::Header msg;
    msg.type     = ICMP::EchoRequest;
    msg.code     = 0;
    msg.checksum = 0;
    msg.id       = 1;
    msg.sequence = 1;

    // Generate checksum
    msg.checksum = IPV4::checksum(&msg, sizeof(msg));

    // Send the packet
    if (::write(sock, &msg, sizeof(msg)) <= 0)
    {
        ERROR("failed to send ICMP request: " << strerror(errno));
        return IOError;
    }
    printf("Sending ICMP request to %s\r\n", host);

    // Receive echo reply
    if (::read(sock, &msg, sizeof(msg)) <= 0)
    {
        ERROR("failed to receive ICMP response: " << strerror(errno));
        return IOError;
    }

    // Check message type
    if (msg.type != ICMP::EchoReply)
    {
        ERROR("invalid ICMP code in response: " << (int) msg.type);
        return IOError;
    }

    // Print the ICMP address received
    printf("Received ICMP response with id=%d sequence=%d\r\n",
            msg.id, msg.sequence);

    // Finished
    ::close(sock);
    return Success;
}
