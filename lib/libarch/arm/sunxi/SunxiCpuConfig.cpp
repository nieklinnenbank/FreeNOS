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

#include <Log.h>
#include "SunxiCpuConfig.h"

SunxiCpuConfig::SunxiCpuConfig()
{
}

SunxiCpuConfig::Result SunxiCpuConfig::initialize()
{
    if (m_io.map(IOBase, PAGESIZE,
                 Memory::User | Memory::Readable |
                 Memory::Writable | Memory::Device) != IO::Success)
    {
        ERROR("failed to map I/O memory");
        return IOError;
    }

    return Success;
}

SunxiCpuConfig::Result SunxiCpuConfig::discover()
{
    for (Size i = 0; i < NumberOfCores; i++)
        m_cores.append(i);

    return Success;
}

SunxiCpuConfig::Result SunxiCpuConfig::boot(CoreInfo *info)
{
    u32 reg = 0;

    DEBUG("resetting core" << info->coreId <<
          " at " << (void*) info->kernelEntry);

    switch (info->coreId)
    {
        case 0: reg = Cpu0RstCtrl; break;
        case 1: reg = Cpu1RstCtrl; break;
        case 2: reg = Cpu2RstCtrl; break;
        case 3: reg = Cpu3RstCtrl; break;
        default: {
            ERROR("coreId " << info->coreId << " is invalid");
            return InvalidArgument;
        }
    }

    m_io.write(EntryAddr, info->kernelEntry);
    m_io.write(reg, CpuCoreReset);
    return IOError;
}
