/**
 * Copyright (C) 2009 Niek Linnenbank
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

#include <api/ProcessCtl.h>
#include <arch/CPU.h>
#include <Macros.h>
#include <Types.h>
#include <Config.h>
#include "i8250Terminal.h"
#include "Terminal.h"

i8250Terminal::i8250Terminal(u16 b, u16 i)
    : Terminal(), base(b), irq(i)
{
    /* Aquire I/O port and IRQ line permissions. */
    ProcessCtl(TERMINAL_PID, AllowIO,  base);
    ProcessCtl(TERMINAL_PID, AllowIO,  base + LINESTATUS);
    ProcessCtl(TERMINAL_PID, AllowIO,  base + LINECONTROL);
    ProcessCtl(TERMINAL_PID, AllowIO,  base + IRQCONTROL);
    ProcessCtl(TERMINAL_PID, AllowIO,  base + FIFOCONTROL);
    ProcessCtl(TERMINAL_PID, AllowIO,  base + MODEMCONTROL);
    ProcessCtl(TERMINAL_PID, AllowIO,  base + DIVISORLOW);
    ProcessCtl(TERMINAL_PID, AllowIO,  base + DIVISORHIGH);
    ProcessCtl(TERMINAL_PID, WatchIRQ, irq);
    
    /* 8bit Words, no parity. */
    outb(base + LINECONTROL, 3);
    
    /* Enable interrupts. */
    outb(base + IRQCONTROL, 1);
    
    /* No FIFO. */
    outb(base + FIFOCONTROL, 0);
    
    /* Data Ready, Request to Send. */
    outb(base + MODEMCONTROL, 3);
    
    /* Set baudrate. */
    outb(base + LINECONTROL, inb(base + LINECONTROL) | DLAB);
    outb(base + DIVISORLOW,  (11500 / BAUDRATE) & 0xff);
    outb(base + DIVISORHIGH, (11500 / BAUDRATE) >> 8);
    outb(base + LINECONTROL, inb(base + LINECONTROL) & ~DLAB);
}

int i8250Terminal::read(s8 *buffer, Size size)
{
    Size bytes = 0;
    
    /* Read as much bytes as possible. */
    while (inb(base + LINESTATUS) & RXREADY && bytes < size)
    {
	buffer[bytes++] = inb(base);
    }
    return (bytes);
}
								     
int i8250Terminal::write(s8 *buffer, Size size)
{
    Size bytes = 0;

    /* Write as much bytes as possible. */
    while (inb(base + LINESTATUS) & TXREADY && bytes < size)
    {
    	outb(base, buffer[bytes++]);
    }
    return (bytes);
}
