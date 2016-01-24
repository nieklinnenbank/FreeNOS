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

#include <FreeNOS/System.h>
#include "PL011.h"

PL011::PL011(u32 irq)
    : Device(CharacterDeviceFile)
{
    m_irq = irq;
    m_identifier << "serial0";
}

Error PL011::initialize()
{
    // Remap IO base to ensure we have user-level access to the registers.
    if (m_io.map(IO_BASE + GPIO_BASE, PAGESIZE*2,
                 Memory::User | Memory::Readable | Memory::Writable | Memory::Device)
        != IO::Success)
        return EINVAL;

    // TODO: hack. disable IRQ_REG() == 0...
    ProcessCtl(SELF, DisableIRQ, 0);

    // Disable PL011.
    m_io.write(PL011_CR, 0x00000000);
    
    // Setup the GPIO pin 14 && 15.
    // Disable pull up/down for all GPIO pins & delay for 150 cycles.
    m_io.write(GPPUD, 0x00000000);
    delay(150);

    // Disable pull up/down for pin 14,15 & delay for 150 cycles.
    m_io.write(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);

    // Write 0 to GPPUDCLK0 to make it take effect.
    m_io.write(GPPUDCLK0, 0x00000000);
    
    // Clear pending interrupts.
    m_io.write(PL011_ICR, 0x7FF);

    // Set integer & fractional part of baud rate.
    // Divider = UART_CLOCK/(16 * Baud)
    // Fraction part register = (Fractional part * 64) + 0.5
    // UART_CLOCK = 3000000; Baud = 115200.

    // Divider = 3000000/(16 * 115200) = 1.627 = ~1.
    // Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
    m_io.write(PL011_IBRD, 1);
    m_io.write(PL011_FBRD, 40);

    // Disable FIFO, use 8 bit data transmission, 1 stop bit, no parity
    m_io.write(PL011_LCRH, PL011_LCRH_WLEN_8BIT);

    // Enable Rx/Tx interrupts
    m_io.write(PL011_IMSC,
         PL011_IMSC_RXIM); // | PL011_IMSC_TXIM);

    // Enable PL011, receive & transfer part of UART.
    m_io.write(PL011_CR, (1 << 0) | (1 << 8) | (1 << 9));
    return 0;
}

Error PL011::interrupt(u32 vector)
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
    ProcessCtl(SELF, EnableIRQ, m_irq);
    return ESUCCESS;
}

Error PL011::read(IOBuffer & buffer, Size size, Size offset)
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
    return buffer.getCount() ? (Error) buffer.getCount() : EAGAIN;
}

Error PL011::write(IOBuffer & buffer, Size size, Size offset)
{
    Size bytes = 0;

    // Clear Transmit Interrupts
    u32 mis = m_io.read(PL011_MIS);
    if (mis & PL011_MIS_TXMIS)
        m_io.write(PL011_ICR, PL011_ICR_TXIC);

#warning if FR_TXFE sets while buffer isnt empty, we need to wait for the TX IRQ, which we disabled!

    // Write as much bytes as possible
    while ((m_io.read(PL011_FR) & PL011_FR_TXFE) && bytes < size)
    {
        m_io.write(PL011_DR, buffer[bytes++]);
    }
    return bytes ? (Error) bytes : EAGAIN;
}

void PL011::delay(s32 count)
{
    asm volatile("1: subs %[count], %[count], #1; bne 1b"
         : : [count]"r"(count));
}
