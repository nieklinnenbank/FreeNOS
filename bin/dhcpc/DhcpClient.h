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

#ifndef __BIN_DHCPC_DHCPCLIENT_H
#define __BIN_DHCPC_DHCPCLIENT_H

#include <NetworkClient.h>
#include <POSIXApplication.h>

/**
 * @addtogroup bin
 * @{
 */

/**
 * Dynamic Host Configuration Protocol (DHCP) client application.
 *
 * @see https://tools.ietf.org/html/rfc1541
 * @see https://tools.ietf.org/html/rfc2131
 * @see https://tools.ietf.org/html/rfc1533
 */
class DhcpClient : public POSIXApplication
{
  private:

    /** Server UDP port */
    static const u16 ServerPort = 67;

    /** Client UDP port */
    static const u16 ClientPort = 68;

    /** Maximum number of retries to receive an IP address */
    static const Size MaximumRetries = 25;

    /** Magic number value for the packet header */
    static const u32 MagicValue = 0x63825363;

    /** Timeout in milliseconds to wait for packet receive */
    static const Size ReceiveTimeoutMs = 500;

    /**
     * Protocol packet header
     */
    struct Header
    {
        u8 operation;
        u8 hardwareType;
        u8 hardwareLength;
        u8 hopCount;
        u32 transactionId;
        u16 seconds;
        u16 flags;
        u32 clientAddress;
        u32 yourAddress;
        u32 serverAddress;
        u32 gatewayAddress;
        u8 clientHardware[16];
        u8 padding[192];
        u32 magic;
    };

    /**
     * DHCP operations
     */
    enum Operation
    {
        BootRequest  = 1,
        BootResponse = 2
    };

    /**
     * DHCP message types
     */
    enum MessageType
    {
        Discover = 1,
        Offer    = 2,
        Request  = 3,
        Decline  = 4,
        Ack      = 5,
        Nak      = 6,
        Release  = 7
    };

    /**
     * DHCP options
     */
    enum Options
    {
        SubnetMask           = 1,
        Router               = 3,
        DomainNameServer     = 6,
        RequestedIP          = 50,
        DhcpMessageType      = 53,
        ServerIdentifier     = 54,
        ParameterRequestList = 55,
        EndMark              = 255
    };

  public:

    /**
     * Class constructor.
     *
     * @param argc Argument count
     * @param argv Argument values
     */
    DhcpClient(int argc, char **argv);

    /**
     * Class destructor.
     */
    virtual ~DhcpClient();

    /**
     * Initialize the application.
     *
     * @return Result code
     */
    virtual Result initialize();

    /**
     * Execute the application event loop.
     *
     * @return Result code
     */
    virtual Result exec();

  private:

    /**
     * Set IP address on a device
     *
     * @param device Name of the network device
     * @param ipAddr IPV4 address to set
     *
     * @return Result code
     */
    Result setIpAddress(const char *device,
                        const IPV4::Address ipAddr) const;

    /**
     * Send DHCP Discover message
     *
     * @param ipAddr Requested IP address
     * @param ipServer Server IP address
     * @param ipGateway Gateway IP address
     *
     * @return Result code
     */
    Result discover(const IPV4::Address &ipAddr,
                    const IPV4::Address &ipServer,
                    const IPV4::Address &ipGateway) const;

    /**
     * Receive DHCP Offer message
     *
     * @param ipAddr Offered IP address on output
     * @param ipServer Server IP address on output
     * @param ipGateway Gateway IP address on output
     *
     * @return Result code
     */
    Result offer(IPV4::Address &ipAddr,
                 IPV4::Address &ipServer,
                 IPV4::Address &ipGateway) const;

    /**
     * Send DHCP Request message
     *
     * @param ipAddr Requested IP address
     * @param ipServer Server IP address
     * @param ipGateway Gateway IP address
     *
     * @return Result code
     */
    Result request(const IPV4::Address &ipAddr,
                   const IPV4::Address &ipServer,
                   const IPV4::Address &ipGateway) const;

    /**
     * Receive DHCP Acknowledge message
     *
     * @param ipAddr Acknowledged IP address on output
     * @param ipServer Server IP address on output
     * @param ipGateway Gateway IP address on output
     *
     * @return Result code
     */
    Result acknowledge(IPV4::Address &ipAddr,
                       IPV4::Address &ipServer,
                       IPV4::Address &ipGateway) const;

    /**
     * Send DHCP boot request
     *
     * @param messageType Value for the DHCP message type option
     * @param ipAddr Requested IP address
     * @param ipServer Server IP address
     * @param ipGateway Gateway IP address
     *
     * @return Result code
     */
    Result sendBootRequest(const DhcpClient::MessageType messageType,
                           const IPV4::Address & ipAddr,
                           const IPV4::Address & ipServer,
                           const IPV4::Address & ipGateway) const;

    /**
     * Receive DHCP boot response
     *
     * @param messageType Expected value for DHCP message type option
     * @param ipAddr IP address on output
     * @param ipServer Server IP address on output
     * @param ipGateway Gateway IP address on output
     *
     * @return Result code
     */
    Result receiveBootResponse(const DhcpClient::MessageType messageType,
                               IPV4::Address &ipAddr,
                               IPV4::Address &ipServer,
                               IPV4::Address &ipGateway) const;

    /**
     * Send UDP broadcast packet
     *
     * @param packet Payload to send
     * @param size Number of bytes to send
     *
     * @return Result code
     */
    Result udpSend(const void *packet,
                   const Size size) const;

    /**
     * Receive UDP packet
     *
     * @param packet Payload output
     * @param size Output for number of bytes received
     *
     * @return Result code
     */
    Result udpReceive(void *packet,
                      Size & size) const;

  private:

    /** Network client */
    NetworkClient *m_client;

    /** UDP socket */
    int m_socket;

    /** Host ethernet address */
    Ethernet::Address m_etherAddress;

    /** Transaction ID of the current request */
    u32 m_transactionId;
};

/**
 * @}
 */

#endif /* __BIN_DHCPC_DHCPCLIENT_H */
