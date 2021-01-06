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

#ifndef __LIBARCH_INTELIO_H
#define __LIBARCH_INTELIO_H

#include <Types.h>
#include <IO.h>

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libarch
 * @{
 *
 * @addtogroup libarch_intel
 * @{
 */

/**
 * Intel I/O functions.
 */
class IntelIO : public IO
{
  public:

    /**
     * Constructor
     */
    IntelIO()
        : IO()
        , m_portBase(0)
    {
    }

    /**
     * Set port I/O base address
     *
     * @param base New I/O base address.
     */
    void setPortBase(const u16 base)
    {
        m_portBase = base;
    }

    /**
     * Read a byte from a port.
     *
     * @param port The I/O port to read from.
     *
     * @return A byte read from the port.
     */
    inline u8 inb(u16 port) const
    {
        u8 b;
        port += m_portBase;
        asm volatile ("inb %%dx, %%al" : "=a" (b) : "d" (port));
        return b;
    }

    /**
     * Read a word from a port.
     *
     * @param port The I/O port to read from.
     *
     * @return Word read from the port.
     */
    inline u16 inw(u16 port) const
    {
        u16 w;
        port += m_portBase;
        asm volatile ("inw %%dx, %%ax" : "=a" (w) : "d" (port));
        return w;
    }

    /**
     * Output a byte to a port.
     *
     * @param port Port to write to.
     * @param byte The byte to output.
     */
    inline void outb(u16 port, u8 byte)
    {
        port += m_portBase;
        asm volatile ("outb %%al,%%dx"::"a" (byte),"d" (port));
    }

    /**
     * Output a word to a port.
     *
     * @param port Port to write to.
     * @param word The word to output.
     */
    inline void outw(u16 port, u16 word)
    {
        port += m_portBase;
        asm volatile ("outw %%ax,%%dx"::"a" (word),"d" (port));
    }

    /**
     * Output a long to a I/O port.
     *
     * @param port Target I/O port.
     * @param l The long 32-bit number to output.
     */
    inline void outl(u16 port, u32 l)
    {
        port += m_portBase;
        asm volatile ("outl %%eax,%%dx"::"a" (l),"d" (port));
    }

    /**
     * Read memory mapped register.
     *
     * @param addr Address of the register to read.
     *
     * @return 32-bit value of the register.
     */
    inline u32 read(const Address addr) const
    {
        const volatile u32 *ptr = (const volatile u32 *)((const volatile u8 *)m_base + addr);
        return *ptr;
    }

    /**
     * Read a number of 32-bit values.
     *
     * @param addr Address of the starting 32-bit value.
     * @param count Number of bytes to read.
     * @param buf Output buffer.
     */
    inline void read(Address addr, Size count, void *buf) const
    {
        for (Size i = 0; i < count; i+= sizeof(u32))
        {
            *(u32 *)(((u8 *)buf) + i) = read(addr + i);
        }
    }

    /**
     * Write memory mapped register.
     *
     * @param addr Address of the register to write.
     * @param data 32-bit value to write in the register.
     */
    inline void write(const Address addr, const u32 data)
    {
        volatile u32 *ptr = (volatile u32 *)((volatile u8 *)m_base + addr);
        *ptr = data;
    }

    /**
     * Write a number of 32-bit values.
     *
     * @param addr Address of the starting 32-bit value.
     * @param count Number of bytes to write.
     * @param buf Input buffer.
     */
    inline void write(Address addr, Size count, const void *buf)
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
    inline void set(Address addr, u32 data)
    {
        u32 current = read(addr);
        current |= data;
        write(addr, current);
    }

    /**
     * Unset bits in memory mapped register.
     *
     * @param addr Address of the register to write.
     * @param data 32-bit value containing the bits to set (bitwise or).
     */
    inline void unset(Address addr, u32 data)
    {
        u32 current = read(addr);
        current &= ~(data);
        write(addr, current);
    }

  private:

    /** Port I/O base address */
    u16 m_portBase;
};

namespace Arch
{
    typedef IntelIO IO;
};

/**
 * @}
 * @}
 * @}
 */

#endif /* __LIBARCH_INTELIO_H */
