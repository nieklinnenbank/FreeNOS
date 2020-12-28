/*
 * Copyright (C) 2015 Niek Linnenbank
 * Copyright (C) 2013 Goswin von Brederlow <goswin-v-b@web.de>
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

#include <FreeNOS/Constant.h>
#include <FreeNOS/User.h>
#include "PL011.h"

template<> SerialDevice* AbstractFactory<SerialDevice>::create()
{
    return new PL011(UART0_IRQ);
}

PL011::PL011(const u32 irq)
    : SerialDevice(irq)
{
    m_identifier << "serial0";
}

FileSystem::Result PL011::initialize()
{
    if (!isKernel)
    {
        // Remap IO base to ensure we have user-level access to the registers.
        if (m_io.map(UART_BASE, PAGESIZE*2,
                     Memory::User | Memory::Readable | Memory::Writable | Memory::Device)
            != IO::Success)
        {
            return FileSystem::IOError;
        }

        // Disable receiving interrupts
        ProcessCtl(SELF, DisableIRQ, m_irq);
    }
    else
    {
        m_io.setBase(UART_BASE);
    }

    // Disable PL011.
    m_io.write(PL011_CR, 0x00000000);

    // Clear pending interrupts.
    m_io.write(PL011_ICR, 0x7FF);

    // Set integer & fractional part of baud rate.
    m_io.write(PL011_IBRD, 26);
    m_io.write(PL011_FBRD, 3);

    // Disable FIFO, use 8 bit data transmission, 1 stop bit, no parity
    m_io.write(PL011_LCRH, PL011_LCRH_WLEN_8BIT);

    if (isKernel)
    {
        // Mask all interrupts.
        m_io.write(PL011_IMSC, (1 << 1) | (1 << 4) | (1 << 5) |
                               (1 << 6) | (1 << 7) | (1 << 8) |
                               (1 << 9) | (1 << 10));
    }
    else
    {
        // Enable Rx/Tx interrupts
        m_io.write(PL011_IMSC, PL011_IMSC_RXIM);
        ProcessCtl(SELF, EnableIRQ, m_irq);
    }

    // Enable PL011, receive & transfer part of UART.
    m_io.write(PL011_CR, (1 << 0) | (1 << 8) | (1 << 9));

    return FileSystem::Success;
}

FileSystem::Result PL011::interrupt(const Size vector)
{
    // Clear Receive Interrupts
    u32 mis = m_io.read(PL011_MIS);
    if (mis & PL011_MIS_RXMIS)
        m_io.write(PL011_ICR, PL011_ICR_RXIC);

    // Clear Transmit Interrupts
    mis = m_io.read(PL011_MIS);
    if (mis & PL011_MIS_TXMIS)
        m_io.write(PL011_ICR, PL011_ICR_TXIC);

    // Re-enable interrupts
    if (!isKernel)
    {
        ProcessCtl(SELF, EnableIRQ, m_irq);
    }

    return FileSystem::Success;
}

FileSystem::Result PL011::read(IOBuffer & buffer,
                               Size & size,
                               const Size offset)
{
    Size bytes = 0;

    // Clear Receive Interrupts
    u32 mis = m_io.read(PL011_MIS);
    if (mis & PL011_MIS_RXMIS)
        m_io.write(PL011_ICR, PL011_ICR_RXIC);

    // Read as much bytes as possible
    while (!(m_io.read(PL011_FR) & PL011_FR_RXFE) && bytes < size)
    {
        //buffer[bytes++] = m_io.read(PL011_DR);
        u8 byte = m_io.read(PL011_DR);
        buffer.bufferedWrite(&byte, 1);
        bytes++;
    }

    if (buffer.getCount())
    {
        size = buffer.getCount();
        return FileSystem::Success;
    }
    else
    {
        return FileSystem::RetryAgain;
    }
}

FileSystem::Result PL011::write(IOBuffer & buffer,
                                Size & size,
                                const Size offset)
{
    Size bytes = 0;

    // Clear Transmit Interrupts
    u32 mis = m_io.read(PL011_MIS);
    if (mis & PL011_MIS_TXMIS)
        m_io.write(PL011_ICR, PL011_ICR_TXIC);

    // Write as much bytes as possible
    while (bytes < size)
    {
        if (m_io.read(PL011_FR) & PL011_FR_TXFE)
        {
            m_io.write(PL011_DR, buffer[bytes++]);
        }
    }

    if (bytes)
    {
        size = bytes;
        return FileSystem::Success;
    }
    else
    {
        return FileSystem::RetryAgain;
    }
}
