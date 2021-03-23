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

#include <Assert.h>
#include "NetworkServer.h"

NetworkServer::NetworkServer(const char *path)
    : DeviceServer(path)
    , m_device(ZERO)
{
}

NetworkServer::~NetworkServer()
{
}

void NetworkServer::registerNetworkDevice(NetworkDevice *dev)
{
    registerDevice(dev, "io");
    m_device = dev;
}

FileSystem::Result NetworkServer::initialize()
{
    DEBUG("");
    return DeviceServer::initialize();
}

void NetworkServer::onProcessTerminated(const ProcessID pid)
{
    DEBUG("pid = " << pid);

    if (m_device != ZERO)
    {
        m_device->unregisterSockets(pid);
    }
}

bool NetworkServer::retryRequests()
{
    assert(m_device != ZERO);

    // Process all pending requests
    while (DeviceServer::retryRequests())
    {
    }

    // Start DMA engine
    const FileSystem::Result result = m_device->startDMA();
    if (result != FileSystem::Success)
    {
        ERROR("failed to start DMA: result = " << (int) result);
    }

    // All requests done
    return false;
}
