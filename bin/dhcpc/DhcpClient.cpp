/*
 * Copyright (C) 2020 Niek Linnenbank
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

#pragma clang optimize off
#pragma GCC push_options
#pragma GCC optimize ("O0")

#include <Log.h>
#include <ByteOrder.h>
#include <MemoryBlock.h>
#include <BufferedFile.h>
#include <NetworkClient.h>
#include <NetworkSocket.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include "DhcpClient.h"

DhcpClient::DhcpClient(int argc, char **argv)
    : POSIXApplication(argc, argv)
    , m_client(ZERO)
    , m_socket(0)
    , m_transactionId(1)
{
    MemoryBlock::set(&m_etherAddress, 0, sizeof(m_etherAddress));

    parser().setDescription("Dynamic Host Configuration Protocol (DHCP) client");
    parser().registerPositional("DEVICE", "device name of network adapter");
}

DhcpClient::~DhcpClient()
{
    delete m_client;
}

DhcpClient::Result DhcpClient::initialize()
{
    const char *device = arguments().get("DEVICE");
    NetworkClient::Result result;

    DEBUG("");

    // Read ethernet device address
    String ethFilePath;
    ethFilePath << "/network/" << device << "/ethernet/address";
    BufferedFile ethFile(*ethFilePath);

    const BufferedFile::Result readResult = ethFile.read();
    if (readResult != BufferedFile::Success)
    {
        ERROR("failed to read ethernet device address of " <<
               device << ": result = " << (int) readResult);
        return IOError;
    }
    MemoryBlock::copy(&m_etherAddress, ethFile.buffer(), sizeof(m_etherAddress));
    DEBUG(device << " has address " << m_etherAddress);

    // Create a network client
    m_client = new NetworkClient(device);

    // Initialize networking client
    result = m_client->initialize();
    if (result != NetworkClient::Success)
    {
        ERROR("failed to initialize network client for device "
               << device << ": result = " << (int) result);
        return IOError;
    }

    // Create an UDP socket
    result = m_client->createSocket(NetworkClient::UDP, &m_socket);
    if (result != NetworkClient::Success)
    {
        ERROR("failed to create UDP socket on device " << device <<
              ": result = " << (int) result);
        return IOError;
    }

    // Bind to a local port.
    result = m_client->bindSocket(m_socket, 0, ClientPort);
    if (result != NetworkClient::Success)
    {
        ERROR("failed to bind socket to UDP port " << ClientPort <<
              " on device " << device << ": result = " << (int) result);
        return IOError;
    }

    // Success
    return Success;
}

DhcpClient::Result DhcpClient::exec()
{
    DEBUG("");

    // Keep retrying until we have an address
    for (Size i = 0; i < MaximumRetries; i++)
    {
        IPV4::Address ipAddr, ipServer, ipGateway;
        DhcpClient::Result result;

        DEBUG("device = " << arguments().get("DEVICE") << " attempt = " << (i + 1));

        m_transactionId++;
        ipAddr = ipServer = ipGateway = 0;

        result = discover(ipAddr, ipServer, ipGateway);
        if (result != DhcpClient::Success)
        {
            ERROR("failed to send discover: result = " << (int) result);
            continue;
        }

        result = offer(ipAddr, ipServer, ipGateway);
        if (result != DhcpClient::Success)
        {
            ERROR("failed to receive offer: result = " << (int) result);
            continue;
        }

        result = request(ipAddr, ipServer, ipGateway);
        if (result != DhcpClient::Success)
        {
            ERROR("failed to send request: result = " << (int) result);
            continue;
        }

        result = acknowledge(ipAddr, ipServer, ipGateway);
        if (result != DhcpClient::Success)
        {
            ERROR("failed to receive acknowledge: result = " << (int) result);
            continue;
        }

        DEBUG("ipAddr = " << *IPV4::toString(ipAddr) <<
              " ipServer = " << *IPV4::toString(ipServer) <<
              " ipGateway = " << *IPV4::toString(ipGateway));

        return setIpAddress(arguments().get("DEVICE"), ipAddr);
    }

    return NotFound;
}

DhcpClient::Result DhcpClient::setIpAddress(const char *device,
                                            const IPV4::Address ipAddr) const
{
    DEBUG("device = " << device << " ipAddr = " << *IPV4::toString(ipAddr));

    // Apply the IP address on the device
    String ipFilePath;
    ipFilePath << "/network/" << device << "/ipv4/address";
    BufferedFile ipFile(*ipFilePath);

    const BufferedFile::Result writeResult = ipFile.write(&ipAddr, sizeof(ipAddr));
    if (writeResult != BufferedFile::Success)
    {
        ERROR("failed to set IPV4 address for device " <<
               device << ": result = " << (int) writeResult);
        return IOError;
    }

    return Success;
}

DhcpClient::Result DhcpClient::discover(const IPV4::Address & ipAddr,
                                        const IPV4::Address & ipServer,
                                        const IPV4::Address & ipGateway) const
{
    DEBUG("");
    return sendBootRequest(Discover, ipAddr, ipServer, ipGateway);
}

DhcpClient::Result DhcpClient::offer(IPV4::Address & ipAddr,
                                     IPV4::Address & ipServer,
                                     IPV4::Address & ipGateway) const
{
    DEBUG("");
    return receiveBootResponse(Offer, ipAddr, ipServer, ipGateway);
}


DhcpClient::Result DhcpClient::request(const IPV4::Address & ipAddr,
                                       const IPV4::Address & ipServer,
                                       const IPV4::Address & ipGateway) const
{
    DEBUG("");
    return sendBootRequest(Request, ipAddr, ipServer, ipGateway);
}

DhcpClient::Result DhcpClient::acknowledge(IPV4::Address & ipAddr,
                                           IPV4::Address & ipServer,
                                           IPV4::Address & ipGateway) const
{
    DEBUG("");
    return receiveBootResponse(Ack, ipAddr, ipServer, ipGateway);
}

DhcpClient::Result DhcpClient::sendBootRequest(const DhcpClient::MessageType messageType,
                                               const IPV4::Address & ipAddr,
                                               const IPV4::Address & ipServer,
                                               const IPV4::Address & ipGateway) const
{
    u8 pkt[1024];
    DhcpClient::Header *hdr = (DhcpClient::Header *) &pkt;

    DEBUG("messageType = " << (int) messageType <<
          " ipAddr = " << *IPV4::toString(ipAddr) <<
          " ipServer = " << *IPV4::toString(ipServer) <<
          " ipGateway = " << *IPV4::toString(ipGateway));

    // Prepare request packet
    MemoryBlock::set(&pkt, 0, sizeof(pkt));
    hdr->operation = BootRequest;
    hdr->hardwareType = 0x1;
    hdr->hardwareLength = 0x6;
    writeBe32(&hdr->transactionId, m_transactionId);
    MemoryBlock::copy(&hdr->clientHardware, &m_etherAddress, sizeof(Ethernet::Address));
    writeBe32(&hdr->magic, MagicValue);

    // Add message type
    u8 *opt = (u8 *) (hdr + 1);
    *opt++ = DhcpMessageType;
    *opt++ = sizeof(u8);
    *opt++ = messageType;

    // Add requested IP
    if (ipAddr != ZERO)
    {
        *opt++ = RequestedIP;
        *opt++ = sizeof(IPV4::Address);
        writeBe32(opt, ipAddr);
        opt += sizeof(IPV4::Address);
    }

    // Add parameter list
    *opt++ = ParameterRequestList;
    *opt++ = 2;
    *opt++ = Router;
    *opt++ = DomainNameServer;

    // Add server IP
    if (ipServer != ZERO)
    {
        *opt++ = ServerIdentifier;
        *opt++ = sizeof(IPV4::Address);
        writeBe32(opt, ipServer);
        opt += sizeof(IPV4::Address);
    }

    // Close options
    *opt++ = EndMark;

    // Send the packet
    return udpSend(&pkt, opt - &pkt[0]);
}

DhcpClient::Result DhcpClient::receiveBootResponse(const DhcpClient::MessageType messageType,
                                                   IPV4::Address & ipAddr,
                                                   IPV4::Address & ipServer,
                                                   IPV4::Address & ipGateway) const
{
    u8 pkt[1024];
    Size size = sizeof(pkt);
    DhcpClient::Header *hdr = (DhcpClient::Header *) &pkt;

    DEBUG("");

    const DhcpClient::Result result = udpReceive(&pkt, size);
    if (result != DhcpClient::Success)
    {
        ERROR("failed to receive UDP packet: result = " << (int) result);
        return IOError;
    }

    // The packet must be targeted at our ethernet address
    if (!MemoryBlock::compare(&hdr->clientHardware,
                              &m_etherAddress, sizeof(Ethernet::Address)))
    {
        ERROR("invalid ethernet address: " << *(Ethernet::Address *) &hdr->clientHardware);
        return InvalidArgument;
    }

    // The packet operation must be a DHCP response
    if (hdr->operation != BootResponse)
    {
        ERROR("invalid operation: " << hdr->operation << " != " << (int) BootResponse);
        return InvalidArgument;
    }

    // The packet must have the correct transaction number
    if (readBe32(&hdr->transactionId) != m_transactionId)
    {
        ERROR("invalid transaction: " << readBe32(&hdr->transactionId) <<
              " != " << m_transactionId);
        return InvalidArgument;
    }

    // Parse the offer message
    ipAddr = readBe32(&hdr->yourAddress);

    // Parse options
    for (u8 *option = (u8 *) (hdr + 1); option < &pkt[size - 2]; )
    {
        const Options optionValue = (const Options) *option++;
        const Size optionLength = *option++;

        if (option + optionLength > &pkt[size])
            break;

        switch (optionValue)
        {
            case Router:
            {
                ipGateway = readBe32(option);
                break;
            }

            case SubnetMask:
            case EndMark:
            case DomainNameServer:
            case RequestedIP:
                break;

            case DhcpMessageType:
            {
                if (messageType != (DhcpClient::MessageType) (*option))
                {
                    ERROR("invalid message type: " << (int) (*option) << " != " << (int) messageType);
                    return InvalidArgument;
                }
            }

            case ParameterRequestList:
                break;

            case ServerIdentifier:
            {
                ipServer = readBe32(option);
                break;
            }

            default:
                DEBUG("ignored unsupported option " << (int) optionValue);
                break;
        }

        option += optionLength;

        if (optionValue == EndMark)
            break;
    }

    DEBUG("messageType = " << (int) messageType <<
          " ipAddr = " << *IPV4::toString(ipAddr) <<
          " ipServer = " << *IPV4::toString(ipServer) <<
          " ipGateway = " << *IPV4::toString(ipGateway));

    return DhcpClient::Success;
}

DhcpClient::Result DhcpClient::udpSend(const void *packet,
                                       const Size size) const
{
    DEBUG("size = " << size);

    // Prepare UDP broadcast datagram
    struct sockaddr addr;
    addr.addr = 0xffffffff;
    addr.port = ServerPort;

    // Send the packet
    int result = ::sendto(m_socket, packet, size, 0,
                          &addr, sizeof(addr));
    if (result <= 0)
    {
        ERROR("failed to send UDP datagram: " << strerror(errno));
        return IOError;
    }

    return Success;
}

DhcpClient::Result DhcpClient::udpReceive(void *packet,
                                          Size & size) const
{
    DEBUG("");

    struct sockaddr addr;

    // Wait for a packet in the UDP socket
    const NetworkClient::Result result = m_client->waitSocket(NetworkClient::UDP, m_socket, ReceiveTimeoutMs);
    if (result != NetworkClient::Success)
    {
        ERROR("failed to wait for UDP socket " << m_socket << ": result = " << (int) result);
        return IOError;
    }

    // Receive UDP datagram
    int r = recvfrom(m_socket, packet, size, 0,
                     &addr, sizeof(addr));
    if (r < 0)
    {
        ERROR("failed to receive UDP datagram: " << strerror(errno));
        return IOError;
    }

    size = r;
    DEBUG("received " << r << " bytes from " << *IPV4::toString(addr.addr) <<
          " at port " << addr.port);

    return Success;
}
