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

#ifndef __LIB_LIBNET_NETWORKDEVICE_H
#define __LIB_LIBNET_NETWORKDEVICE_H

#include <Types.h>
#include <Device.h>
#include "Ethernet.h"
#include "ARP.h"
#include "IPV4.h"
#include "ICMP.h"
#include "UDP.h"
#include "NetworkQueue.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libnet
 * @{
 */

/**
 * Network Device abstract class.
 */
class NetworkDevice : public Device
{
  public:

    /**
     * Constructor
     *
     * @param inode Inode number
     * @param server NetworkServer reference
     */
    NetworkDevice(const u32 inode,
                  NetworkServer &server);

    /**
     * Destructor
     */
    virtual ~NetworkDevice();

    /**
     * Initialize the device
     *
     * @return Result code
     */
    virtual FileSystem::Result initialize();

    /**
     * Get maximum packet size
     *
     * @return Maximum packet size
     */
    const Size getMaximumPacketSize() const;

    /**
     * Read ethernet address.
     *
     * @param address Ethernet address reference for output
     *
     * @return Result code
     */
    virtual FileSystem::Result getAddress(Ethernet::Address *address) = 0;

    /**
     * Set ethernet address
     *
     * @param address New ethernet address to set
     *
     * @return Result code
     */
    virtual FileSystem::Result setAddress(const Ethernet::Address *address) = 0;

    /**
     * Get receive queue
     */
    NetworkQueue * getReceiveQueue();

    /**
     * Get transmit queue
     */
    NetworkQueue * getTransmitQueue();

    /**
     * Remove sockets for a process
     *
     * @param pid ProcessID to remove sockets for
     */
    void unregisterSockets(const ProcessID pid);

    /**
     * Add a network packet to the transmit queue.
     *
     * @param packet Pointer to the packet to transmit
     *
     * @return Result code
     */
    virtual FileSystem::Result transmit(NetworkQueue::Packet *packet) = 0;

    /**
     * Process a received network packet.
     *
     * @param packet Network packet received by the device
     * @param offset Network packet payload offset
     *
     * @return Result code
     */
    virtual FileSystem::Result process(const NetworkQueue::Packet *packet,
                                       const Size offset = 0);

    /**
     * Start DMA processing
     *
     * @return Result code
     */
    virtual FileSystem::Result startDMA();

  protected:

    /** Maximum size of each packet */
    Size m_maximumPacketSize;

    NetworkQueue m_receive;

    NetworkQueue m_transmit;

    NetworkServer &m_server;

    Ethernet *m_eth;

    ARP *m_arp;

    IPV4 *m_ipv4;

    ICMP *m_icmp;

    UDP *m_udp;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBNET_NETWORKDEVICE_H */
