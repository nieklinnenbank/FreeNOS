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

#ifndef __LIB_LIBNET_NETWORKPROTOCOL_H
#define __LIB_LIBNET_NETWORKPROTOCOL_H

#include <Directory.h>
#include <Types.h>
#include "NetworkQueue.h"

class NetworkServer;
class NetworkDevice;

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libnet
 * @{
 */

/**
 * Network protocol abstraction class.
 */
class NetworkProtocol : public Directory
{
  protected:

    /**
     * List of known network protocol identifiers
     */
    enum Identifier
    {
        Ethernet = 1,
        IPV4,
        ARP,
        ICMP,
        UDP,
        TCP
    };

  public:

    /**
     * Constructor
     *
     * @param server Reference to the NetworkServer instance
     * @param device Reference to the NetworkDevice instance
     * @param parent Parent upper-layer protocol (or use this for no parent)
     */
    NetworkProtocol(NetworkServer &server,
                    NetworkDevice &device,
                    NetworkProtocol &parent);

    /**
     * Destructor
     */
    virtual ~NetworkProtocol();


    /**
     * Get maximum packet size.
     *
     * @return Packet size in bytes
     */
    virtual const Size getMaximumPacketSize() const;

    /**
     * Perform initialization.
     *
     * @return Result code
     */
    virtual FileSystem::Result initialize() = 0;

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
                                       const Size offset) = 0;

  protected:

    /** Network server instance */
    NetworkServer &m_server;

    /** Network device instance */
    NetworkDevice &m_device;

    /** Parent upper-layer protocol instance */
    NetworkProtocol &m_parent;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBNET_NETWORKPROTOCOL_H */
