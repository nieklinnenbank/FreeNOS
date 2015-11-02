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

#include <Log.h>
#include "BroadcomI2C.h"

BroadcomI2C::BroadcomI2C()
    : I2C()
{
}

BroadcomI2C::Result BroadcomI2C::initialize()
{
    // Map I2C registers
    if (m_io.map(IO_BASE + BSC1, PAGESIZE,
                 Memory::User|Memory::Readable|Memory::Writable|Memory::Device) != IO::Success)
        return IOError;

    // Initialize GPIO controller
    if (m_gpio.initialize() != BroadcomGPIO::Success)
        return IOError;

    // Set GPIO pins to ALT0 function for SDA, SCL pins
    m_gpio.setAltFunction(2, BroadcomGPIO::Function0);
    m_gpio.setAltFunction(3, BroadcomGPIO::Function0);
    NOTICE("I2C GPIO pins set");
    NOTICE("ClockDivider is " << m_io.read(ClockDivider));
    NOTICE("Status is " << m_io.read(Status));

    // Done
    return Success;
}

BroadcomI2C::Result BroadcomI2C::setAddress(Address addr)
{
    DEBUG("address is now " << addr);
    m_io.write(SlaveAddress, addr);
    return Success;
}

BroadcomI2C::Result BroadcomI2C::write(u8 *buf, Size size)
{
    if (size > FIFOSize)
        return RangeError;

    // Clear FIFO and Status
    m_io.write(Control, ClearFIFO);
    m_io.write(Status, ClockStretchTimeout | AcknowledgeError | TransferDone);

    // Set data length
    m_io.write(DataLength, size);

    DEBUG("Writing " << size << " bytes");

    // Fill the FIFO
    for (Size i = 0; i < size; i++)
    {
        DEBUG("buf[" << i << "] = " << buf[i]);
        m_io.write(FIFO, buf[i]);
    }

    // Begin transfer
    m_io.write(Control, Enable | Transfer);

    // Wait until transfer done is set
    while (!(m_io.read(Status) & TransferDone));

    // Check status
    if (m_io.read(Status) & AcknowledgeError)
    {
        ERROR("Acknowledge error");
        return IOError;
    }
    else if (m_io.read(Status) & ClockStretchTimeout)
    {
        ERROR("ClockStretchTimeout error");
        return IOError;
    }
    // Done
    return Success;
}

BroadcomI2C::Result BroadcomI2C::read(u8 *buf, Size size)
{
    Size count = 0;

    if (size > FIFOSize)
        return RangeError;

    // Clear FIFO and Status
    m_io.write(Control, ClearFIFO);
    m_io.write(Status, ClockStretchTimeout | AcknowledgeError | TransferDone);

    // Set data length
    m_io.write(DataLength, size);

    // Begin transfer
    m_io.write(Control, Enable | ReadMode | Transfer);

    // Wait until transfer done is set
    while (!(m_io.read(Status) & TransferDone))
    {
        while (m_io.read(Status) & RxFIFOHasData)
            buf[count++] = m_io.read(FIFO);
    }
    // Check status
    if (m_io.read(Status) & AcknowledgeError)
    {
        ERROR("Acknowledge error");
        return IOError;
    }
    else if (m_io.read(Status) & ClockStretchTimeout)
    {
        ERROR("ClockStretchTimeout error");
        return IOError;
    }
    // Done
    return Success;
}
