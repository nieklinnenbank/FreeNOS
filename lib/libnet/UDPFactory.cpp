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

#include "UDP.h"
#include "UDPFactory.h"
#include "UDPSocket.h"

UDPFactory::UDPFactory(UDP *udp)
{
    m_udp = udp;
}

UDPFactory::~UDPFactory()
{
}

Error UDPFactory::read(IOBuffer & buffer, Size size, Size offset)
{
    DEBUG("");

    String path;
    UDPSocket *sock;

    if (offset > 0)
        return 0;

    sock = m_udp->createSocket(path);
    if (!sock)
        return EIO;

    buffer.write(*path, path.length() + 1);
    return path.length() + 1;
}
