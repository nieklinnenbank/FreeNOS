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

#include <Log.h>
#include <String.h>
#include <MemoryBlock.h>
#include "IPV4Address.h"

IPV4Address::IPV4Address(const u32 inode,
                         IPV4 *ipv4)
    : File(inode)
    , m_ipv4(ipv4)
{
    m_size = sizeof(IPV4::Address);
}

IPV4Address::~IPV4Address()
{
}

FileSystem::Result IPV4Address::read(IOBuffer & buffer,
                                     Size & size,
                                     const Size offset)
{
    IPV4::Address addr;
    m_ipv4->getAddress(&addr);

    if (offset >= m_size)
    {
        size = 0;
        return FileSystem::Success;
    }

    buffer.write(&addr, sizeof(addr));
    size = sizeof(addr);

    return FileSystem::Success;
}

FileSystem::Result IPV4Address::write(IOBuffer & buffer,
                                      Size & size,
                                      const Size offset)
{
    IPV4::Address addr;
    char tmp[32];

    buffer.read(tmp, size < sizeof(tmp) ? size : sizeof(tmp));
    tmp[sizeof(tmp) - 1] = 0;

    // Try to convert from a text dotted notation to IPV4::Address
    addr = IPV4::toAddress(tmp);
    if (!addr)
    {
        // If not in text format, it must be given in 32-bit value
        if (size != sizeof(IPV4::Address))
        {
            ERROR("invalid IP addresss format given");
            return FileSystem::InvalidArgument;
        }
        MemoryBlock::copy(&addr, tmp, sizeof(addr));
    }

    DEBUG("address = " << *IPV4::toString(addr));

    // Set the address
    return m_ipv4->setAddress(&addr);
}
