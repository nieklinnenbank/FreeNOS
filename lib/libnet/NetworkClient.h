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

#ifndef __LIB_LIBNET_NETWORKCLIENT_H
#define __LIB_LIBNET_NETWORKCLIENT_H

#include <Types.h>
#include "IPV4.h"
#include "Ethernet.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libnet
 * @{
 */

/**
 * Networking Client implementation.
 *
 * Maintains also the network state of each TCP/UDP socket, creates the
 * packets and posts them to ethernet. Also receives packets and extracts socket info.
 *
 * Because of the client and multiplexer separation the networking stack
 * is very parallel. It can scale very well for multicore/manycore systems
 * as each client may run on a different (possibly dedicated using pinning) core
 * thus running multiple network stack instances on each core in parallel.
 */
class NetworkClient
{
  public:

    /**
     * Socket actions
     */
    enum SocketAction
    {
        Connect,
        Listen,
        SendSingle,
        SendMultiple
    };

    /**
     * Socket information
     *
     * This struct is written to a socket
     * to put it in either connect or listen state.
     */
    typedef struct SocketInfo
    {
        IPV4::Address address;
        u16 port;
        u16 action;
    }
    SocketInfo;

    /**
     * Describes a single packet.
     *
     * This structure is used for operations that involve multiple packets,
     * for example: SendMultiple.
     */
    struct PacketInfo
    {
        Address address;
        Size size;
    };

    /**
     * Socket types
     */
    enum SocketType
    {
        ARP,
        ICMP,
        TCP,
        UDP
    };

    /**
     * Result codes
     */
    enum Result
    {
        Success,
        IOError,
        NotFound,
        NotSupported,
        TimedOut
    };

  public:

    /**
     * Constructor
     *
     * @param networkDevice Name of the network device to use
     */
    NetworkClient(const char *networkDevice);

    /**
     * Destructor
     */
    virtual ~NetworkClient();

    /**
     * Perform initialization.
     *
     * @return Result code
     */
    Result initialize();

    /**
     * Create new socket.
     *
     * @param type Type of the socket to create
     * @param socket Outputs a pointer to NetworkSocket
     *
     * @return Result code
     */
    Result createSocket(const SocketType type,
                        int *socket);

    /**
     * Connect socket to address/port.
     *
     * @param sock Socket index
     * @param addr Address of the host to connect to
     * @param port Port of the host to connect to (or ZERO to ignore)
     *
     * @return Result code
     */
    Result connectSocket(const int sock,
                         const IPV4::Address addr,
                         const u16 port = 0);

    /**
     * Bind socket to address/port.
     *
     * @param sock Socket index
     * @param addr Address of the address to bind to
     * @param port Port to bind to
     *
     * @return Result code
     */
    Result bindSocket(const int sock,
                      const IPV4::Address addr = 0,
                      const u16 port = 0);

    /**
     * Wait until the given socket has data to receive.
     *
     * @param type Type of socket to wait for
     * @param sock Socket index
     * @param msecTimeout Timeout in milliseconds to wait or ZERO for infinite wait
     *
     * @return Result code
     */
    Result waitSocket(const NetworkClient::SocketType type,
                      const int sock,
                      const Size msecTimeout);

    /**
     * Close the socket.
     *
     * @param sock Network socket to close
     *
     * @return Result code
     */
    Result close(const int sock);

  private:

    /**
     * Set socket to new state.
     */
    Result writeSocketInfo(const int sock,
                           const IPV4::Address addr,
                           const u16 port,
                           const SocketAction action);

  private:

    /** Network device name */
    String m_deviceName;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBNET_NETWORKCLIENT_H */
