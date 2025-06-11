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
#include "PL011.h"

PL011::PL011()
{
}

bool PL011::initialize()
{
    m_io.setBase(UART_BASE);

    // Disable PL011.
    m_io.write(PL011_CR, 0x00000000);

    // Clear pending interrupts.
    m_io.write(PL011_ICR, 0x7FF);

    // Set integer & fractional part of baud rate.
    m_io.write(PL011_IBRD, 26);
    m_io.write(PL011_FBRD, 3);

    // Disable FIFO, use 8 bit data transmission, 1 stop bit, no parity
    m_io.write(PL011_LCRH, PL011_LCRH_WLEN_8BIT);

    // Mask all interrupts.
    m_io.write(PL011_IMSC, (1 << 1) | (1 << 4) | (1 << 5) |
                           (1 << 6) | (1 << 7) | (1 << 8) |
                           (1 << 9) | (1 << 10));

    // Enable PL011, receive & transfer part of UART.
    m_io.write(PL011_CR, (1 << 0) | (1 << 8) | (1 << 9));

    return true;
}


void PL011::write(const char *buffer, const Size size)
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
}
