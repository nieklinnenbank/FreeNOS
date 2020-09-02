/*
 * Copyright (C) 2020 Niek Linnenbank
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

#ifndef __LIBARCH_HOST_HOSTIO_H
#define __LIBARCH_HOST_HOSTIO_H

#include <Types.h>
#include "IO.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libarch
 * @{
 *
 * @addtogroup libarch_host
 * @{
 */

/**
 * Generic Input/Output operations suitable for the host operating system.
 */
class HostIO : public IO
{
  public:

    /**
     * write to memory mapped I/O register
     */
    inline void write(const u32 reg, const u32 data)
    {
        volatile u32 *ptr = (volatile u32 *)((volatile u8 *)m_base + reg);
        *ptr = data;
    }

    /**
     * read from memory mapped I/O register
     *
     * @param reg Address to read
     *
     * @return 32-bit value
     */
    inline u32 read(const u32 reg) const
    {
        const volatile u32 *ptr = (const volatile u32 *)((const volatile u8 *)m_base + reg);
        return *ptr;
    }

    /**
     * Read a number of 32-bit values.
     *
     * @param addr Address of the starting 32-bit value.
     * @param count Number of bytes to read.
     * @param buf Output buffer.
     */
    inline void read(const Address addr, const Size count, void *buf) const
    {
        for (Size i = 0; i < count; i+= sizeof(u32))
        {
            *(u32 *)(((u8 *)buf) + i) = read(addr + i);
        }
    }

    /**
     * Write a number of 32-bit values.
     *
     * @param addr Address of the starting 32-bit value.
     * @param count Number of bytes to write.
     * @param buf Input buffer.
     */
    inline void write(const Address addr, const Size count, const void *buf)
    {
        for (Size i = 0; i < count; i+= sizeof(u32))
        {
            write(addr + i, *(u32 *) (((u8 *)buf) + i));
        }
    }

    /**
     * Set bits in memory mapped register.
     *
     * @param addr Address of the register to write.
     * @param data 32-bit value containing the bits to set (bitwise or).
     */
    inline void set(const Address addr, const u32 data)
    {
        volatile u32 current = read(addr);
        current |= data;
        write(addr, current);
    }

    /**
     * Unset bits in memory mapped register.
     *
     * @param addr Address of the register to write.
     * @param data 32-bit value containing the bits to set (bitwise or).
     */
    inline void unset(const Address addr, const u32 data)
    {
        volatile u32 current = read(addr);
        current &= ~(data);
        write(addr, current);
    }
};

namespace Arch
{
    typedef HostIO IO;
};

/**
 * @}
 * @}
 * @}
 */

#endif /* __LIBARCH_HOST_HOSTIO_H */
