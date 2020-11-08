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

#ifndef __LIB_LIBNET_ARPSOCKET_H
#define __LIB_LIBNET_ARPSOCKET_H

#include "IPV4.h"
#include "NetworkSocket.h"
#include "NetworkQueue.h"

class ARP;

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libnet
 * @{
 */

/**
 * Address Resolution Protocol (ARP) socket.
 *
 * ARP sockets receive a copy of ARP reply packets.
 */
class ARPSocket : public NetworkSocket
{
  public:

    /**
     * Constructor
     *
     * @param inode Inode number
     * @param arp ARP object pointer
     */
    ARPSocket(const u32 inode,
              ARP *arp);

    /**
     * Destructor
     */
    virtual ~ARPSocket();

    /**
     * Receive ARP response
     *
     * @param buffer Input/Output buffer to output bytes to.
     * @param size Maximum number of bytes to read on input.
     *             On output, the actual number of bytes read.
     * @param offset Offset inside the file to start reading.
     *
     * @return Result code
     */
    virtual FileSystem::Result read(IOBuffer & buffer,
                                    Size & size,
                                    const Size offset);

    /**
     * Send ARP request
     *
     * @param buffer Input/Output buffer to input bytes from.
     * @param size Maximum number of bytes to write on input.
     *             On output, the actual number of bytes written.
     * @param offset Offset inside the file to start writing.
     *
     * @return Result code
     */
    virtual FileSystem::Result write(IOBuffer & buffer,
                                     Size & size,
                                     const Size offset);

    /**
     * Process incoming network packet.
     *
     * @param pkt Incoming packet pointer
     *
     * @return Result code
     */
    virtual FileSystem::Result process(const NetworkQueue::Packet *pkt);

  private:

    /** ARP protocol instance */
    ARP *m_arp;

    /** IPV4 address for request. */
    IPV4::Address m_ipAddr;

    /** Ethernet address for reply. */
    Ethernet::Address m_ethAddr;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBNET_ARPSOCKET_H */
