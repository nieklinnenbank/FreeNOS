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

#include <NetworkClient.h>
#include <sys/socket.h>
#include <errno.h>

extern C int socket(int domain, int type, int protocol)
{
    NetworkClient client("");
    int sock;

    // This opens /network/$device/$protocol/socket
    //
    // The bind() function will then write 0+address+port to that socket file, ( or 'b'+address+port)
    // and read from it to receive the path to the actual file for the new socket,
    // and replaces this fd with that file.
    //
    // connect() does similar, but it will write 1+address+port to that socket file, ( or 'l' + address+port)
    // and read to get the path to the new socket file and replaces the fd.
    client.initialize();

    if (domain == AF_INET && type == SOCK_DGRAM && protocol == IPPROTO_ICMP)
    {
        if (client.createSocket(NetworkClient::ICMP, &sock) == NetworkClient::Success)
            return ESUCCESS;
        else
            return EINVAL;
    }
    return ENOTSUP;
}
