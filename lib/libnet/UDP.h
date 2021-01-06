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

#ifndef __LIB_LIBNET_UDP_H
#define __LIB_LIBNET_UDP_H

#include <Types.h>
#include <Index.h>
#include <String.h>
#include <HashTable.h>
#include "NetworkProtocol.h"
#include "UDPSocket.h"

class UDPFactory;

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
     *
     * @param server Reference to the NetworkServer instance
     * @param device Reference to the NetworkDevice instance
     * @param parent Parent upper-layer protocol
     */
    UDP(NetworkServer &server,
        NetworkDevice &device,
        NetworkProtocol &parent);

    /**
     * Destructor
     */
    virtual ~UDP();

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
    UDPSocket * createSocket(String & path,
                             const ProcessID pid);

    /**
     * Remove sockets for a process
     *
     * @param pid ProcessID to remove sockets for
     */
    void unregisterSockets(const ProcessID pid);

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
    FileSystem::Result sendPacket(const NetworkClient::SocketInfo *src,
                                  const NetworkClient::SocketInfo *dest,
                                  IOBuffer & buffer,
                                  const Size size,
                                  const Size offset);

    /**
     * Calculate ICMP checksum
     *
     * @param ip Pointer to the IPV4 header to use
     * @param header ICMP header
     * @param datalen Total number of bytes of the payload data
     *
     * @return ICMP checksum value for the given header
     */
    static const u16 checksum(const IPV4::Header *ip,
                              const Header *header,
                              const Size datalen);

  private:

    /**
     * Calculate sum of artibrary data
     *
     * @param ptr Pointer to the data to sum
     * @param bytes Number of bytes to add to the sum
     *
     * @return Sum value for the input data
     */
    static const ulong calculateSum(const u16 *ptr,
                                    const Size bytes);

  private:

    /** Factory for creating new UDP sockets */
    UDPFactory *m_factory;

    /** Contains all UDP sockets */
    Index<UDPSocket, MaxUdpSockets> m_sockets;

    /** Maps UDP ports to UDP sockets */
    HashTable<u16, UDPSocket *> m_ports;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBNET_UDP_H */
