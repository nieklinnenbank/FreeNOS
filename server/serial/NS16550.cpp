/*
 * Copyright (C) 2019 Niek Linnenbank
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
#include "NS16550.h"

template<> SerialDevice* AbstractFactory<SerialDevice>::create()
{
    return new NS16550(UART0_IRQ);
}

NS16550::NS16550(const u32 irq)
    : SerialDevice(irq)
{
    m_identifier << "serial0";
}

FileSystem::Result NS16550::initialize()
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

    // Set integer & fractional part of baud rate.
    setDivisorLatch(true);
    m_io.write(DivisorLatchLow, 13);
    m_io.write(DivisorLatchHigh, 0x00);
    setDivisorLatch(false);

    // Use 8 bit data transmission, 1 stop bit, no parity
    m_io.write(LineControl, LineControl8Bits);

    if (isKernel)
    {
        // Mask all interrupts.
        m_io.write(InterruptEnable, 0);
    }
    else
    {
        // Enable Rx/Tx interrupts and FIFOs
        m_io.write(FifoControl, FifoControlTrigger1 | FifoControlEnable);
        m_io.write(InterruptEnable, ReceiveDataInterrupt);
        ProcessCtl(SELF, EnableIRQ, m_irq);
    }

    return FileSystem::Success;
}

FileSystem::Result NS16550::interrupt(const Size vector)
{
    // Mask interrupt until FIFOs are empty
    m_io.write(InterruptEnable, 0);
    return FileSystem::Success;
}

FileSystem::Result NS16550::read(IOBuffer & buffer,
                                 Size & size,
                                 const Size offset)
{
    Size bytes = 0;

    // Read as much bytes as possible
    while ((m_io.read(LineStatus) & LineStatusDataReady) && bytes < size)
    {
        u8 byte = m_io.read(ReceiveBuffer);
        buffer.bufferedWrite(&byte, 1);
        bytes++;
    }

    // Re-enable interrupts
    if (!isKernel)
    {
        if (!(m_io.read(InterruptEnable) & ReceiveDataInterrupt))
        {
            m_io.write(InterruptEnable, ReceiveDataInterrupt);
            ProcessCtl(SELF, EnableIRQ, m_irq);
        }
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

FileSystem::Result NS16550::write(IOBuffer & buffer,
                                  Size & size,
                                  const Size offset)
{
    Size bytes = 0;

    // Write as much bytes as possible
    while (bytes < size)
    {
        // Wait until TX fifo is empty
        while (!(m_io.read(LineStatus) & LineStatusTxEmpty))
        {
            ;
        }

        m_io.write(TransmitHolding, buffer[bytes++]);
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

void NS16550::setDivisorLatch(bool enabled)
{
    // Set the divisor latch register
    u32 lc = m_io.read(LineControl);

    if (enabled)
        lc |= LineControlDivisorLatch;
    else
        lc &= ~LineControlDivisorLatch;

    m_io.write(LineControl, lc);
}
