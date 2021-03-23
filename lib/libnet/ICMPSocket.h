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

#ifndef __LIB_LIBNET_ICMPSOCKET_H
#define __LIB_LIBNET_ICMPSOCKET_H

#include "NetworkSocket.h"
#include "NetworkQueue.h"
#include "NetworkClient.h"

class ICMP;

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libnet
 * @{
 */

/**
 * Internet Control Message Protocol (ICMP) socket.
 *
 * ICMP sockets are associated with one IP-host and
 * receive packets from that host and allow sending packets
 * to that host.
 */
class ICMPSocket : public NetworkSocket
{
  public:

    /**
     * Constructor
     *
     * @param inode Inode number
     * @param icmp ICMP object pointer
     * @param pid ProcessID owning this socket
     */
    ICMPSocket(const u32 inode,
               ICMP *icmp,
               const ProcessID pid);

    /**
     * Destructor
     */
    virtual ~ICMPSocket();

    /**
     * Get associated IP host
     *
     * @return IPV4::Address of the connected host
     */
    const IPV4::Address getAddress() const;

    /**
     * Read ICMP response
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
     * Send ICMP request
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

    /**
     * Set ICMP reply
     *
     * @param reply ICMP reply header
     */
    void setReply(const ICMP::Header *reply);

  private:

    /** ICMP protocol instance */
    ICMP *m_icmp;

    /** Reply */
    ICMP::Header m_reply;
    bool m_gotReply;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBNET_ICMPSOCKET_H */
