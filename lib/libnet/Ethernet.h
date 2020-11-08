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

#ifndef __LIB_LIBNET_ETHERNET_H
#define __LIB_LIBNET_ETHERNET_H

#include <Types.h>
#include <Macros.h>
#include <Log.h>
#include <String.h>
#include "NetworkProtocol.h"

class ARP;
class IPV4;
class ICMP;

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libnet
 * @{
 */

/**
 * Ethernet networking protocol
 */
class Ethernet : public NetworkProtocol
{
  public:

    /** Size of the CRC checksum (which is set after the payload) */
    static const Size CRCSize = 4;

    /**
     * Ethernet network address
     */
    typedef struct Address
    {
        u8 addr[6];
    }
    PACKED Address;

    /**
     * Ethernet network packet header.
     *
     * This header is prepended to every
     * ethernet packet send on the actual physical link.
     */
    typedef struct Header
    {
        Address destination;    /**< packet destination address */
        Address source;         /**< packet source address */
        u16 type;               /**< payload type */
    }
    PACKED Header;

    /**
     * List of ethernet payload types.
     */
    enum PayloadType
    {
        IPV4 = 0x0800u,         /**< Internet protocol v4 */
        IPV6 = 0x86ddu,         /**< Internet protocol v6 */
        ARP  = 0x0806u          /**< Address resolution protocol */
    };

  public:

    /**
     * Constructor
     *
     * @param server Reference to the NetworkServer instance
     * @param device Reference to the NetworkDevice instance
     */
    Ethernet(NetworkServer &server,
             NetworkDevice &device);

    /**
     * Destructor
     */
    virtual ~Ethernet();

    /**
     * Perform initialization.
     *
     * @return Result code
     */
    virtual FileSystem::Result initialize();

    /**
     * Retrieve Ethernet address
     *
     * @param address Output buffer to store the address
     *
     * @return Result code
     */
    virtual FileSystem::Result getAddress(Address *address);

    /**
     * Set Ethernet address
     *
     * @param address Input address
     *
     * @return Result code
     */
    virtual FileSystem::Result setAddress(const Address *address);

    /**
     * Get a new packet for transmission
     *
     * @param pkt On output contains a pointer to a Packet
     * @param address Address of the destination of this packet
     * @param addressSize Number of bytes of the address
     * @param protocol Identifier for the protocol to create the packet for
     * @param payloadSize Number of payload bytes
     *
     * @return Result code
     */
    virtual FileSystem::Result getTransmitPacket(NetworkQueue::Packet **pkt,
                                                 const void *address,
                                                 const Size addressSize,
                                                 const Identifier protocol,
                                                 const Size payloadSize);

    /**
     * Convert address to string
     *
     * @param address Input ethernet address
     * @return Text value of the ethernet address
     */
    static const String toString(const Address address);

    /**
     * Set ARP instance
     *
     * @param arp ARP object instance
     */
    void setARP(::ARP *arp);

    /**
     * Set IPV4 instance
     *
     * @param ip IPV4 instance
     */
    void setIP(::IPV4 *ip);

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

  private:

    /** Current ethernet address */
    Address m_address;

    /** ARP protocol */
    ::ARP *m_arp;

    /** IPV4 protocol */
    ::IPV4 *m_ipv4;
};

Log & operator << (Log &log, const Ethernet::Address & addr);

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBNET_ETHERNET_H */
