/*
 * Copyright (C) 2021 Niek Linnenbank
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

#ifndef __BIN_NETSEND_NETSEND_H
#define __BIN_NETSEND_NETSEND_H

#include <NetworkClient.h>
#include <NetworkQueue.h>
#include <POSIXApplication.h>

/**
 * @addtogroup bin
 * @{
 */

/**
 * Diagnostic program for sending network packets
 */
class NetSend : public POSIXApplication
{
  private:

    /** Size of each packet to send in bytes */
    static const Size PacketSize = 1448;

    /** Number of packets to submit for transmission each iteration */
    static const Size QueueSize = NetworkQueue::MaxPackets;

  public:

    /**
     * Class constructor.
     *
     * @param argc Argument count
     * @param argv Argument values
     */
    NetSend(int argc,
            char **argv);

    /**
     * Class destructor.
     */
    virtual ~NetSend();

    /**
     * Initialize the application.
     *
     * @return Result code
     */
    virtual Result initialize();

    /**
     * Execute the application event loop.
     *
     * @return Result code
     */
    virtual Result exec();

  private:

    /**
     * Send multiple UDP packets
     *
     * @param vec I/O vector with multiple packets
     * @param count Number of entries in the I/O vector
     * @param addr The destination IP and port
     *
     * @return Result code
     */
    Result udpSendMultiple(const struct iovec *vec,
                           const Size count,
                           const struct sockaddr & addr) const;

  private:

    /** Network client */
    NetworkClient *m_client;

    /** UDP socket */
    int m_socket;
};

/**
 * @}
 */

#endif /* __BIN_NETSEND_NETSEND_H */
