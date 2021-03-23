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

#ifndef __SERVER_SERIAL_I8250_H
#define __SERVER_SERIAL_I8250_H

#include <Macros.h>
#include <Types.h>
#include "SerialDevice.h"

/**
 * @addtogroup server
 * @{
 *
 * @addtogroup serial
 * @{
 */

/**
 * i8250 serial UART.
 */
class i8250 : public SerialDevice
{
  private:

    /**
     * Constants used to communicate with the UART.
     */
    enum Constants
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
        DLAB         = 0x80,
        BAUDRATE     = 9600,
    };

  public:

    /**
     * Constructor function.
     *
     * @param irq Interrupt vector
     * @param base I/O base port
     */
    i8250(const u32 irq, const u16 base);

    /**
     * Initializes the i8250 serial UART.
     *
     * @return Result code
     */
    virtual FileSystem::Result initialize();

    /**
     * Called when an interrupt has been triggered for this device.
     *
     * @param vector Vector number of the interrupt.
     *
     * @return Result code
     */
    virtual FileSystem::Result interrupt(const Size vector);

    /**
     * Read bytes from the device
     *
     * @param buffer Input/Output buffer to output bytes to.
     * @param size Maximum number of bytes to read on input.
     *             On output, the actual number of bytes read.
     * @param offset Offset inside the file to start reading.
     *
     * @return Result code
     */
    virtual FileSystem::Result read(IOBuffer & buffer,
                                    Size & size,
                                    const Size offset);

    /**
     * Write bytes to the device
     *
     * @param buffer Input/Output buffer to input bytes from.
     * @param size Maximum number of bytes to write on input.
     *             On output, the actual number of bytes written.
     * @param offset Offset inside the file to start writing.
     *
     * @return Result code
     */
    virtual FileSystem::Result write(IOBuffer & buffer,
                                     Size & size,
                                     const Size offset);
};

/**
 * @}
 * @}
 */

#endif /* __SERVER_SERIAL_I8250_H */
