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

#include <FreeNOS/System.h>
#include "IntelSerial.h"

IntelSerial::IntelSerial(u16 base)
{
    m_base = base;

    /* 8bit Words, no parity. */
    IO::outb(base + LINECONTROL, 3);
    
    /* Enable interrupts. */
    IO::outb(base + IRQCONTROL, 1);
    
    /* No FIFO. */
    IO::outb(base + FIFOCONTROL, 0);
    
    /* Data Ready, Request to Send. */
    IO::outb(base + MODEMCONTROL, 3);
    
    /* Set baudrate. */
    IO::outb(base + LINECONTROL, IO::inb(base + LINECONTROL) | DLAB);
    IO::outb(base + DIVISORLOW,  (11500 / BAUDRATE) & 0xff);
    IO::outb(base + DIVISORHIGH, (11500 / BAUDRATE) >> 8);
    IO::outb(base + LINECONTROL, IO::inb(base + LINECONTROL) & ~DLAB);
}

void IntelSerial::write(const char *str)
{
    /* Write as much bytes as possible. */
    while (IO::inb(m_base + LINESTATUS) & TXREADY && *str)
    {
        IO::outb(m_base, *str++);
    }
}
