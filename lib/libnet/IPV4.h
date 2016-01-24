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

#ifndef __LIBNET_IPV4_H
#define __LIBNET_IPV4_H

#include <Types.h>
#include "Ethernet.h"
#include "IPV4.h"

/**
 * Internet Protocol Version 4
 */
namespace IPV4
{
    /**
     * IP-address
     */
    typedef u32 Address;

    /**
     * Protocol types
     */
    enum Protocol
    {
        ICMP = 1,
        IGMP = 2,
        UDP  = 17,
        TCP  = 6
    };

    /**
     * IP network packet header.
     *
     * This header is prepended to every
     * IP packet send on the actual physical link.
     */
    typedef struct Header
    {
        u8  version:4;
        u8  headerLength:4;
        u8  typeOfService;
        u16 length;
        u16 identification;
        u16 fragmentOffset;
        u8  timeToLive;
        u8  protocol;
        u16 checksum;
        Address source;
        Address destination;
    }
    PACKED Header;
};

#endif /* __LIBNET_IPV4_H */
