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

#include "SunxiPowerManagement.h"

SunxiPowerManagement::Result SunxiPowerManagement::initialize()
{
    if (m_io.map(IOBase & ~0xfff, PAGESIZE,
                 Memory::User | Memory::Readable |
                 Memory::Writable | Memory::Device) != IO::Success)
    {
        ERROR("failed to map I/O memory");
        return IOError;
    }

    m_io.setBase(m_io.getBase() + (IOBase & 0xfff));
    return Success;
}

SunxiPowerManagement::Result SunxiPowerManagement::powerOnCore(const Size coreId)
{
    if (coreId > NumberOfCores)
    {
        ERROR("invalid coreId " << coreId);
        return InvalidArgument;
    }

    // Open up the power clamp for this core
    for (u32 val = 0x1ff; val != 0; val >>= 1)
    {
        m_io.write(CpuPowerClamp + (coreId * sizeof(u32)), val);
    }

    // Now enable the power for the core
    m_io.unset(CpuPowerOff, 1 << coreId);
    return Success;
}
