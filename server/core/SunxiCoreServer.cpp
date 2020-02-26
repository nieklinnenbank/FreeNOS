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

#include <FreeNOS/System.h>
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
    API::Result r = ProcessCtl(SELF, WatchIRQ, IPIVector);

    if (r != API::Success)
    {
        ERROR("failed to register IPI vector: "
              "ProcessCtl(WatchIRQ) returned: " << (uint)r);
        return IOError;
    }

    return CoreServer::initialize();
}

SunxiCoreServer::Result SunxiCoreServer::bootCore(uint coreId, CoreInfo *info)
{
    if (m_cpuConfig.boot(info) != SunxiCpuConfig::Success)
    {
        ERROR("failed to boot coreId" << coreId);
        return BootError;
    }

    return Success;
}

SunxiCoreServer::Result SunxiCoreServer::discoverCores()
{
    if (m_cpuConfig.discover() != SunxiCpuConfig::Success)
    {
        ERROR("failed to discover cores");
        return IOError;
    }

    return Success;
}

void SunxiCoreServer::waitIPI() const
{
    // Wait for IPI which will wake us
    ProcessCtl(SELF, EnableIRQ, IPIVector);
    ProcessCtl(SELF, EnterSleep, 0, 0);
}

SunxiCoreServer::Result SunxiCoreServer::sendIPI(uint coreId)
{
    ERROR("not yet implemented");
    return IOError;
}
