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

#include <FreeNOS/User.h>
#include <FreeNOS/API.h>
#include <Log.h>
#include <Factory.h>
#include "SunxiCoreServer.h"

template<> CoreServer* AbstractFactory<CoreServer>::create()
{
    return new SunxiCoreServer();
}

SunxiCoreServer::SunxiCoreServer()
    : CoreServer()
    , m_cpuConfig()
{
    m_cores = &m_cpuConfig;
}

SunxiCoreServer::Result SunxiCoreServer::initialize()
{
    SunxiCpuConfig::Result cpuResult;
    SystemInformation info;

    API::Result r = ProcessCtl(SELF, WatchIRQ, SoftwareInterruptNumber);
    if (r != API::Success)
    {
        ERROR("failed to register SGI vector: "
              "ProcessCtl(WatchIRQ) returned: " << (uint)r);
        return IOError;
    }

    cpuResult = m_cpuConfig.initialize();
    if (cpuResult != SunxiCpuConfig::Success)
    {
        ERROR("failed to initialize CPU configuration module: " <<
              (uint) cpuResult);
        return IOError;
    }

    // When running as a secondary core, flag ourselves as booted
    if (info.coreId != 0)
    {
        CoreInfo tmpInfo;
        VMCopy(SELF, API::Read, (Address) &tmpInfo, SecondaryCoreInfoAddress, sizeof(tmpInfo));
        tmpInfo.booted = 1;
        VMCopy(SELF, API::Write, (Address) &tmpInfo, SecondaryCoreInfoAddress, sizeof(tmpInfo));
    }

    return CoreServer::initialize();
}

Core::Result SunxiCoreServer::bootCore(uint coreId, CoreInfo *info)
{
    // Calculate the memory location of the CoreInfo structure passed to the
    // secondary core. Note that the location is relative to the info->memory.phys address
    const Address secondaryCoreInfoRelAddr = info->memory.phys + SecondaryCoreInfoOffset;

    // Copy the CoreInfo structure as input for the secondary core.
    // The first copy is used when setting up the early-MMU and the
    // second copy is passed as input to the kernel.
    VMCopy(SELF, API::Write, (Address) info, SecondaryCoreInfoAddress, sizeof(*info));
    VMCopy(SELF, API::Write, (Address) info, secondaryCoreInfoRelAddr, sizeof(*info));

    // Reset the secondary core
    if (m_cpuConfig.boot(info) != SunxiCpuConfig::Success)
    {
        ERROR("failed to boot coreId" << coreId);
        return Core::BootError;
    }

    // Wait until the core raises the 'booted' flag in CoreInfo
    while (1)
    {
        CoreInfo check;

        VMCopy(SELF, API::Read, (Address) &check, secondaryCoreInfoRelAddr, sizeof(check));

        if (check.booted)
            break;
    }

    return Core::Success;
}

Core::Result SunxiCoreServer::discoverCores()
{
    if (m_cpuConfig.discover() != SunxiCpuConfig::Success)
    {
        ERROR("failed to discover cores");
        return Core::IOError;
    }

    return Core::Success;
}

void SunxiCoreServer::waitIPI() const
{
    // Wait for IPI which will wake us
    ProcessCtl(SELF, EnableIRQ, SoftwareInterruptNumber);
    ProcessCtl(SELF, EnterSleep, 0, 0);
}

Core::Result SunxiCoreServer::sendIPI(uint coreId)
{
    API::Result r = ProcessCtl(SELF, SendIRQ, (coreId << 16) | SoftwareInterruptNumber);
    if (r != API::Success)
    {
        ERROR("failed to send IPI to core" << coreId << ": " << (uint)r);
        return Core::IOError;
    }

    return Core::Success;
}
