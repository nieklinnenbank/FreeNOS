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

#ifndef __SERVER_SERIAL_PL011_H
#define __SERVER_SERIAL_PL011_H

#include <Log.h>
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
 * The PL011 is a commonly available UART device frequently found in ARM systems.
 *
 * @see libarch_arm
 */
class PL011 : public SerialDevice
{
  private:

    /**
     * The offsets and flags for each register in the UART.
     */
    enum {
        PL011_DR        = (0x00),
        PL011_RSRECR    = (0x04),

        PL011_FR        = (0x18),
        PL011_FR_RXFE   = (1 << 4),
        PL011_FR_TXFE   = (1 << 7),

        PL011_ILPR      = (0x20),
        PL011_IBRD      = (0x24),
        PL011_FBRD      = (0x28),

        PL011_LCRH      = (0x2C),
        PL011_LCRH_WLEN_8BIT = (0b11<<5),

        PL011_CR        = (0x30),
        PL011_IFLS      = (0x34),

        PL011_IMSC      = (0x38),
        PL011_IMSC_RXIM = (1 << 4),
        PL011_IMSC_TXIM = (1 << 5),

        PL011_RIS       = (0x3C),

        PL011_MIS       = (0x40),
        PL011_MIS_RXMIS = (1 << 4),
        PL011_MIS_TXMIS = (1 << 5),

        PL011_ICR       = (0x44),
        PL011_ICR_TXIC  = (1 << 5),
        PL011_ICR_RXIC  = (1 << 4),

        PL011_DMACR     = (0x48),
        PL011_ITCR      = (0x80),
        PL011_ITIP      = (0x84),
        PL011_ITOP      = (0x88),
        PL011_TDR       = (0x8C)
    };

  public:

    /**
     * Constructor.
     */
    PL011(const u32 irq);

    /**
     * Initializes the UART.
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

#endif /* __SERVER_SERIAL_PL011_H */
