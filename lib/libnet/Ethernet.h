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

#ifndef __LIBNET_ETHERNET_H
#define __LIBNET_ETHERNET_H

#include <Types.h>
#include <Macros.h>

/**
 * Ethernet networking protocol
 */
namespace Ethernet
{
    /** Size of the CRC checksum (which is set after the payload) */
    static const Size CRCSize = 4;

    /**
     * Ethernet network address
     */
    typedef struct Address
    {
        u8 addr[6];
    }
    PACKED Address;

    /**
     * Ethernet network packet header.
     *
     * This header is prepended to every
     * ethernet packet send on the actual physical link.
     */
    typedef struct Header
    {
        Address destination;    /**< packet destination address */
        Address source;         /**< packet source address */
        u16 type;               /**< payload type */
    }
    PACKED Header;

    /**
     * List of ethernet payload types.
     */
    enum PayloadType
    {
        IPV4 = 0x0800u,         /**< Internet protocol v4 */
        IPV6 = 0x86ddu,         /**< Internet protocol v6 */
        ARP  = 0x0806u          /**< Address resolution protocol */
    };
};

#endif /* __LIBNET_ETHERNET_H */
