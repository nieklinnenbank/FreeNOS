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

#include "SynopsisUSB.h"

SynopsisUSB::SynopsisUSB(const char *path)
    : USBController(path)
{
}

Error SynopsisUSB::initialize()
{
    // Map USB host controller registers
    if (m_io.map(IO_BASE + Base, PAGESIZE*2,
                 Memory::User|Memory::Readable|Memory::Writable|Memory::Device) != IO::Success)
    {
        ERROR("failed to map I/O registers");
        return EIO;
    }

    // Check device ID
    if (m_io.read(VendorId) != DefaultVendorId)
    {
        ERROR("incompatible vendorId: " << m_io.read(VendorId) << " != " << DefaultVendorId);
        return EIO;
    }

    DEBUG("UserId: " << m_io.read(UserId) << " VendorId: " << m_io.read(VendorId));
    NOTICE("Synopsis Design Ware USB-on-the-go Host Controller found");

    // Initialize power manager
    if (m_power.initialize() != BroadcomPower::Success)
    {
        ERROR("failed to initialize power manager");
        return EIO;
    }
    // Power on the USB subsystem
    if (m_power.enable(BroadcomPower::USB) != BroadcomPower::Success)
    {
        ERROR("failed to power on the USB subsystem");
        return EIO;
    }
    DEBUG("powered on");

    // Soft-Reset

    // Setup DMA

    // Setup interrupts

    return DeviceServer::initialize();
}
