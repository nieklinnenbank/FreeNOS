/*
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

#ifndef __SERIAL_SERIALSERVER_H
#define __SERIAL_SERIALSERVER_H

#include <IPCServer.h>
#include <DeviceMessage.h>
#include <Types.h>
#include <Error.h>
#include "i8250.h"

/**
 * An I/O port and IRQ number combination.
 */
typedef struct SerialAddress
{
    /** I/O port. */
    u16 port;
    
    /** Interrupt Request (IRQ) number. */
    u16 irq;
    
    /** Pointer to an UART, if any detected. */
    i8250 *uart;
}
SerialAddress;

/**
 * Serial device server.
 */
class SerialServer : public IPCServer<SerialServer, DeviceMessage>
{
    public:
    
	/**
	 * Class constructor function.
	 */
	SerialServer();

    private:
    
	/** List of known serial port/irq combinations. */
	static SerialAddress uarts[];
};

#endif /* __SERIAL_SERIALSERVER_H */
