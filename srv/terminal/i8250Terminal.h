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

#ifndef __TERMINAL_I8250TERMINAL_H
#define __TERMINAL_I8250TERMINAL_H

#include <Macros.h>
#include <Types.h>
#include "Terminal.h"

/**
 * Constants used to communicate with the UART.
 */
enum
{
    TRANSMIT     = 0,
    RECEIVE      = 0,
    DIVISORLOW   = 0,
    DIVISORHIGH  = 1,
    RXREADY      = 1,
    IRQCONTROL   = 1,
    IRQSTATUS    = 2,
    FIFOCONTROL  = 2,
    LINECONTROL  = 3,
    MODEMCONTROL = 4,
    LINESTATUS   = 5,
    TXREADY      = 0x20,
    DLAB	 = 0x80,
    BAUDRATE     = 9600,
    PORT         = 0x3f8,
    IRQ		 = 4,
};

/**
 * Use the i8250 as system terminal.
 */
class i8250Terminal : public Terminal
{
    public:

	/**
	 * Constructor function.
	 * @param base I/O base port.
	 */
        i8250Terminal(u16 base = PORT, u16 irq = IRQ);

	/**
	 * Retrieve the base I/O port.
	 * @return Base I/O port.
	 */	
	u16 getBase()
	{
	    return base;
	}

	/**
	 * Returns our IRQ number.
	 * @return IRQ number.
	 * @return Zero on success and non-zero on failure.
	 */
	u16 getIRQ()
	{
	    return irq;
	}

	/** 
	 * Read bytes from the i8250.
	 * @param buffer Buffer to save the read bytes.
	 * @param size Number of bytes to read.
	 * @return Number of bytes on success and ZERO on failure. 
	 */
	int read(s8 *buffer, Size size);

	/** 
	 * Write bytes to the device.
	 * @param buffer Buffer containing bytes to write. 
	 * @param size Number of bytes to write. 
	 * @return Number of bytes on success and ZERO on failure. 
	 */
	int write(s8 *buffer, Size size);

    private:
    
	/** Base I/O port. */
	u16 base;
	
	/** IRQ of the i8250. */
	u16 irq;
};

#endif /* __TERMINAL_I8250TERMINAL_H */
