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

#ifndef __LIB_LIBNET_NETWORKQUEUE_H
#define __LIB_LIBNET_NETWORKQUEUE_H

#include <Types.h>
#include <Index.h>
#include <Memory.h>
#include <Log.h>

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libnet
 * @{
 */

/**
 * Networking packet queue implementation.
 */
class NetworkQueue
{
  public:

    /** Size of payload memory buffer */
    static const Size PayloadBufferSize = 2048;

    /** Maximum number of packets available */
    static const Size MaxPackets = 64u;

    /**
     * Represents a network packet
     */
    typedef struct Packet
    {
        Size size;
        u8 *data;
    }
    Packet;

  public:

    /**
     * Constructor
     *
     * @param packetSize The size of each packet in bytes
     * @param queueSize The size of the queue in number of packets
     */
    NetworkQueue(const Size packetSize,
                 const Size queueSize = MaxPackets);

    /**
     * Destructor
     */
    virtual ~NetworkQueue();

    /**
     * Get unused packet
     */
    Packet * get();

    /**
     * Put unused packet back.
     */
    void release(Packet *packet);

    /**
     * Enqueue packet with data.
     */
    void push(Packet *packet);

    /**
     * Retrieve packet with data.
     */
    Packet * pop();

    /**
     * Check if data packets are available
     *
     * @return Boolean
     */
    bool hasData() const;

  private:

    /** Contains unused packets */
    Index<Packet, MaxPackets> m_free;

    /** Contains packets with data */
    Index<Packet, MaxPackets> m_data;

    /** Defines the memory range of mapped payload data */
    Memory::Range m_payloadRange;
};

Log & operator << (Log &log, const NetworkQueue::Packet & pkt);

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBNET_NETWORKQUEUE_H */
