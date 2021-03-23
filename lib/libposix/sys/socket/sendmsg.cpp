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

#include <NetworkClient.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>

extern C int sendmsg(int sockfd, const struct msghdr *msg, int flags)
{
    static u8 buf[1024];
    NetworkClient::SocketInfo *info = (NetworkClient::SocketInfo *) buf;
    NetworkClient::PacketInfo *pkt = (NetworkClient::PacketInfo *) (info + 1);
    const struct sockaddr *addr = (const struct sockaddr *) msg->msg_name;
    Size bytes = sizeof(*info);

    if (msg->msg_namelen != sizeof(struct sockaddr))
    {
        return ERANGE;
    }

    info->address = addr->addr;
    info->port = addr->port;
    info->action = NetworkClient::SendMultiple;

    // Prepare the array of NetworkClient::PacketInfo structs
    for (Size i = 0; i < msg->msg_iovlen && bytes < sizeof(buf); i++)
    {
        pkt->address = (Address) msg->msg_iov[i].iov_base;
        pkt->size = msg->msg_iov[i].iov_len;

        bytes += sizeof(*pkt);
        pkt++;
    }

    return ::write(sockfd, buf, bytes);
}
