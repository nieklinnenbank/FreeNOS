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

#ifndef __LIBPOSIX_SYS_SOCKET_H
#define __LIBPOSIX_SYS_SOCKET_H

#include <Macros.h>
#include "types.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libposix
 * @{
 */

/**
 * Defines a socket address and port pair
 */
struct sockaddr
{
    u32 addr;
    u16 port;
};

typedef Size socklen_t;

/**
 * Connect a socket to an address/port.
 *
 * @param sockfd Socket file descriptor
 * @param addr Pointer to the address and port to connect to.
 * @param addrlen Length of the address
 *
 * @return Zero on success and -1 on error
 */
extern C int connect(int sockfd, struct sockaddr *addr, socklen_t addrlen);

/**
 * Receive a single datagram from a socket.
 *
 * @param sockfd Socket file descriptor
 * @param buf Destination buffer for receiving the payload
 * @param len Maximum number of bytes to receive
 * @param flags Optional flags for the receive operation
 * @param addr Pointer to the address and port of the sender of the datagram
 * @param addrlen Length of the address
 *
 * @return Number of bytes received on success and -1 on error
 */
extern C int recvfrom(int sockfd, void *buf, size_t len, int flags,
                      struct sockaddr *addr, socklen_t addrlen);

/**
 * Send a single datagram to a remote host.
 *
 * @param sockfd Socket file descriptor
 * @param buf Source buffer containing the payload to send
 * @param len Maximum number of bytes to send.
 * @param flags Optional flags for the send operation
 * @param addr Pointer to the destination address and port to send to
 * @param addrlen Length of the address
 *
 * @return Number of bytes send on success and -1 on error
 */
extern C int sendto(int sockfd, const void *buf, size_t len, int flags,
                    const struct sockaddr *addr, socklen_t addrlen);

/**
 * @}
 * @}
 */

#endif /* __LIBPOSIX_SYS_SOCKET_H */
