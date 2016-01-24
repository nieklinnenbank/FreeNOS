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

#ifndef __LIBNET_ARP_H
#define __LIBNET_ARP_H

#include <Types.h>
#include "Ethernet.h"
#include "IPV4.h"

/**
 * Address Resolution Protocol
 */
namespace ARP
{
    /**
     * ARP hardware types
     */
    enum HardwareType
    {
        Ethernet = 1
    };

    /**
     * ARP inter-network protocol types.
     */
    enum ProtocolType
    {
        IPV4 = 0x0800
    };

    /**
     * ARP message types (operation codes)
     */
    enum Operation
    {
        Request = 1,
        Reply   = 2
    };

    /**
     * ARP network packet header.
     *
     * This header is prepended to every
     * ARP packet send on the actual physical link.
     */
    typedef struct Header
    {
        u16 hardwareType;
        u16 protocolType;
        u8  hardwareLength;
        u8  protocolLength;
        u16 operation;

        Ethernet::Address etherSource;
        IPV4::Address     ipSource;

        Ethernet::Address etherDestination;
        IPV4::Address     ipDestination;
    }
    PACKED Header;
};

#endif /* __LIBNET_ARP_H */
