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

FileSystem::Result UDPFactory::read(IOBuffer & buffer,
                                    Size & size,
                                    const Size offset)
{
    DEBUG("");

    String path;
    UDPSocket *sock;

    if (offset > 0)
    {
        size = 0;
        return FileSystem::Success;
    }

    sock = m_udp->createSocket(path);
    if (!sock)
    {
        ERROR("failed to create UDP socket");
        return FileSystem::IOError;
    }

    buffer.write(*path, path.length() + 1);
    size = path.length() + 1;
    return FileSystem::Success;
}
