/*
 * Copyright (C) 2020 Niek Linnenbank
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

#ifndef __SERVER_SERIAL_SERIALDEVICE_H
#define __SERVER_SERIAL_SERIALDEVICE_H

#include <FreeNOS/System.h>
#include <Types.h>
#include <Device.h>
#include <Factory.h>

/**
 * @addtogroup server
 * @{
 *
 * @addtogroup serial
 * @{
 */

/**
 * Provides sequential byte stream of incoming (RX) and outgoing (TX) data.
 */
class SerialDevice : public Device,
                     public AbstractFactory<SerialDevice>
{
  public:

    /**
     * Constructor
     *
     * @param irq Interrupt vector
     */
    SerialDevice(const u32 irq);

    /**
     * Get interrupt vector
     *
     * @return Interrupt vector
     */
    u32 getIrq() const;

    /** Keeps track of inode number for SerialDevices */
    static u32 inodeNumber;

  protected:

    /** interrupt vector */
    const u32 m_irq;

    /** I/O instance */
    Arch::IO m_io;
};

/**
 * @}
 * @}
 */

#endif /* __SERVER_SERIAL_NS16550_H */
