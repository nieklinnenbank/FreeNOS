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

#ifndef __SERVER_SERIAL_NS16550_H
#define __SERVER_SERIAL_NS16550_H

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
 * The NS16550 is a commonly available UART device.
 *
 * @see libarch_arm
 */
class NS16550 : public SerialDevice
{
  private:

    /**
     * Hardware registers
     */
    enum Registers
    {
        ReceiveBuffer     = 0x00, /* 0 */
        TransmitHolding   = 0x00,
        DivisorLatchLow   = 0x00,
        DivisorLatchHigh  = 0x04, /* 1 */
        InterruptEnable   = 0x04,
        InterruptIdentity = 0x08, /* 2 */
        FifoControl       = 0x08,
        LineControl       = 0x0C, /* 3 */
        ModemControl      = 0x10, /* 4 */
        LineStatus        = 0x14, /* 5 */
        ModemStatus       = 0x18, /* 6 */
        Scratch           = 0x1C, /* 7 */
        UartStatus        = 0x7C, /* 31 */
        TransmitFifoLvl   = 0x80
    };

    enum InterruptEnableFlags
    {
        ReceiveDataInterrupt = (1 << 0)
    };

    enum InterruptIdentityFlags
    {
        InterruptIdentityFifoEnable = (0x3 << 6)
    };

    enum FifoControlFlags
    {
        FifoControlTrigger1 = (0),
        FifoControlEnable   = (1 << 0)
    };

    enum LineControlFlags
    {
        LineControlDivisorLatch = (1 << 7),
        LineControl8Bits        = (0x3 << 0)
    };

    enum LineStatusFlags
    {
        LineStatusTxEmpty   = (1 << 6),
        LineStatusDataReady = (1 << 0)
    };

    enum UartStatusFlags
    {
        UartStatusReceiveAvailable = (1 << 2),
        UartStatusTransmitAvailable = (1 << 1),
        UartStatusBusy = (1 << 0)
    };

  public:

    /**
     * Constructor.
     */
    NS16550(const u32 irq);

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

  private:

    /**
     * Enable access to the divisor latch registers
     *
     * @param enabled True to enable, false otherwise
     */
    void setDivisorLatch(bool enabled);
};

/**
 * @}
 * @}
 */

#endif /* __SERVER_SERIAL_NS16550_H */
