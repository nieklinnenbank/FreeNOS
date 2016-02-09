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
#include "IPV4Address.h"

IPV4Address::IPV4Address(IPV4 *ipv4)
{
    m_ipv4 = ipv4;
}

IPV4Address::~IPV4Address()
{
}

Error IPV4Address::read(IOBuffer & buffer, Size size, Size offset)
{
    IPV4::Address addr;
    String str;

    m_ipv4->getAddress(&addr);
    str << IPV4::toString(addr);

    if (offset >= str.length())
        return 0;

    DEBUG("address = " << *str);

    buffer.write(*str, str.length());
    return str.length();
}

Error IPV4Address::write(IOBuffer & buffer, Size size, Size offset)
{
    IPV4::Address addr;
    char tmp[32];
    Error r;

    buffer.read(tmp, size < sizeof(tmp) ? size : sizeof(tmp));
    tmp[sizeof(tmp) - 1] = 0;

    DEBUG("address = " << tmp);

    // Try to convert text dotted notation to IPV4::Address
    addr = IPV4::toAddress(tmp);
    if (!addr)
        return ERANGE;

    // Set the address
    if ((r = m_ipv4->setAddress(&addr)) == ESUCCESS)
        return size;
    else
        return r;
}
