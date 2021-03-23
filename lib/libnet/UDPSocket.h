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

#ifndef __LIB_LIBNET_UDPSOCKET_H
#define __LIB_LIBNET_UDPSOCKET_H

#include <Types.h>
#include "NetworkSocket.h"
#include "NetworkQueue.h"
#include "NetworkClient.h"

class UDP;

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libnet
 * @{
 */

/**
 * User Datagram Protocol (UDP) socket.
 *
 * UDP sockets accept payloads to send when writing
 * and read payloads when receiving payloads.
 */
class UDPSocket : public NetworkSocket
{
  public:

    /**
     * Constructor
     */
    UDPSocket(const u32 inode,
              UDP *udp,
              const ProcessID pid);

    /**
     * Destructor
     */
    virtual ~UDPSocket();

    /**
     * Get associated local port.
     *
     * @return Local port
     */
    const u16 getPort() const;

    /**
     * Receive UDP data
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
     * Send UDP data
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
     * Check if the File has data ready for reading.
     *
     * When this function returns true, it can be read without blocking.
     *
     * @return Boolean
     */
    virtual bool canRead() const;

    /**
     * Process incoming network packet.
     *
     * @param pkt Incoming packet pointer
     *
     * @return Result code
     */
    virtual FileSystem::Result process(const NetworkQueue::Packet *pkt);

  private:

    /** UDP protocol instance */
    UDP *m_udp;

    /** Local port */
    u16 m_port;

    /** Incoming packet queue */
    NetworkQueue m_queue;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBNET_UDPSOCKET_H */
