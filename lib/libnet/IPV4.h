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

#ifndef __LIB_LIBNET_IPV4_H
#define __LIB_LIBNET_IPV4_H

#include <Types.h>
#include <String.h>
#include "NetworkProtocol.h"

class ICMP;
class ARP;
class UDP;

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libnet
 * @{
 */

/**
 * Internet Protocol Version 4
 */
class IPV4 : public NetworkProtocol
{
  public:

    /**
     * IP-address
     */
    typedef u32 Address;

    /**
     * Protocol types
     */
    enum Protocol
    {
        ICMP = 1,
        IGMP = 2,
        UDP  = 17,
        TCP  = 6
    };

    /**
     * IP network packet header.
     *
     * This header is prepended to every
     * IP packet send on the actual physical link.
     */
    typedef struct Header
    {
        u8  versionIHL; /**< Version and header length (IHL) */
        u8  typeOfService;
        u16 length;
        u16 identification;
        u16 fragmentOffset;
        u8  timeToLive;
        u8  protocol;
        u16 checksum;
        Address source;
        Address destination;
    }
    PACKED Header;

    /**
     * Pseudo Header.
     *
     * This header can be used by sub-protocols for
     * checksum calculation (e.g. TCP and UDP).
     */
    typedef struct PseudoHeader
    {
        Address source;
        Address destination;
        u8      reserved;
        u8      protocol;
        u16     length;
    }
    PseudoHeader;

  public:

    /**
     * Constructor
     *
     * @param server Reference to the NetworkServer instance
     * @param device Reference to the NetworkDevice instance
     * @param parent Parent upper-layer protocol
     */
    IPV4(NetworkServer &server,
         NetworkDevice &device,
         NetworkProtocol &parent);

    /**
     * Destructor
     */
    virtual ~IPV4();

    /**
     * Perform initialization.
     *
     * @return Result code
     */
    virtual FileSystem::Result initialize();

    /**
     * Set ICMP instance
     *
     * @param icmp ICMP instance
     */
    void setICMP(::ICMP *icmp);

    /**
     * Set ARP instance
     *
     * @param arp ARP instance
     */
    void setARP(::ARP *arp);

    /**
     * Set UDP instance
     *
     * @param udp UDP instance
     */
    void setUDP(::UDP *udp);

    /**
     * Get current IP address
     *
     * @param address IPV4 address buffer
     *
     * @return Result code
     */
    virtual FileSystem::Result getAddress(Address *address);

    /**
     * Set current IP address
     *
     * @param address IPV4 address buffer
     *
     * @return Result code
     */
    virtual FileSystem::Result setAddress(const Address *address);

    /**
     * Convert address to string
     *
     * @param address Input IP address
     *
     * @return Text value of the IP
     */
    static const String toString(const Address address);

    /**
     * Convert string to IPV4 address.
     *
     * @param address Input textual IP address in dot notation (xxx.xxx.xxx.xxx)
     * @return IPV4::Address or ZERO if not valid
     */
    static const Address toAddress(const char *address);

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
     * Calculate IP checksum
     *
     * @param buffer Input buffer for calculating checksum
     * @param length Number of bytes in the buffer
     *
     * @return IP checksum value for the given buffer
     */
    static const u16 checksum(const void *buffer,
                              const Size length);

  private:

    /**
     * Convert protocol identifier
     *
     * @param id NetworkProtocol identifier
     *
     * @return Protocol value for IPV4 header
     */
    Protocol getProtocolByIdentifier(const NetworkProtocol::Identifier id) const;

  private:

    /** Current IP address */
    Address m_address;

    /** ICMP instance */
    ::ICMP *m_icmp;

    /** ARP instance */
    ::ARP *m_arp;

    /** UDP instance */
    ::UDP *m_udp;

    /** Packet ID for IPV4 */
    u16 m_id;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBNET_IPV4_H */
