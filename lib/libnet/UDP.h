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

#ifndef __LIBNET_UDP_H
#define __LIBNET_UDP_H

#include <Types.h>
#include <Index.h>
#include <String.h>
#include <HashTable.h>
#include "NetworkProtocol.h"
#include "IPV4.h"
#include "UDPSocket.h"

class UDPFactory;
class ARP;

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libnet
 * @{
 */

/**
 * User Datagram Protocol (UDP)
 */
class UDP : public NetworkProtocol
{
  private:

    static const Size MaxUdpSockets = 128u;

  public:

    /**
     * Packet header format
     */
    typedef struct Header
    {
        u16 sourcePort;
        u16 destPort;
        u16 length;
        u16 checksum;
    }
    Header;

  public:

    /**
     * Constructor
     */
    UDP(NetworkServer *server,
        NetworkDevice *device);

    /**
     * Destructor
     */
    virtual ~UDP();

    /**
     * Set IP object
     */
    void setIP(::IPV4 *ip);

    /**
     * Perform initialization.
     *
     * @return Result code
     */
    virtual FileSystem::Result initialize();

    /**
     * Creates an UDP socket
     *
     * @return UDPSocket object instance
     */
    UDPSocket * createSocket(String & path);

    /**
     * Process incoming network packet.
     *
     * @param pkt Incoming packet pointer
     * @param offset Offset for processing
     *
     * @return Result code
     */
    virtual FileSystem::Result process(const NetworkQueue::Packet *pkt,
                                       const Size offset);

    /**
     * Bind to UDP port
     *
     * @param sock UDP socket
     * @param port The port to bind to
     *
     * @return Result code
     */
    FileSystem::Result bind(UDPSocket *sock,
                            const u16 port);

    /**
     * Send packet
     *
     * @return Result code
     */
    FileSystem::Result sendPacket(const NetworkClient::SocketInfo *info,
                                  IOBuffer & buffer,
                                  const Size size);

    /**
     * Calculate ICMP checksum
     *
     * @param header ICMP header
     * @return ICMP checksum value for the given header
     */
    static const u16 checksum(const IPV4::Header *ip,
                              const Header *header,
                              const Size datalen);

  private:

    static const ulong calculateSum(const u16 *ptr,
                                    const Size bytes);

  private:

    UDPFactory *m_factory;

    Index<UDPSocket, MaxUdpSockets> m_sockets;

    /** Maps UDP ports to UDP sockets */
    HashTable<u16, UDPSocket *> m_ports;

    ::IPV4 *m_ipv4;
};

/**
 * @}
 * @}
 */

#endif /* __LIBNET_UDP_H */
