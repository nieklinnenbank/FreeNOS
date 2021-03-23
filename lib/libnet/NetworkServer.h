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

#ifndef __LIBNET_NETWORKSERVER_H
#define __LIBNET_NETWORKSERVER_H

#include <Types.h>
#include <DeviceServer.h>
#include "NetworkDevice.h"
#include "Ethernet.h"
#include "IPV4.h"

class NetworkDevice;

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libnet
 * @{
 */

/**
 * Networking server.
 */
class NetworkServer : public DeviceServer
{
  public:

    /**
     * Constructor
     */
    NetworkServer(const char *path);

    /**
     * Destructor
     */
    virtual ~NetworkServer();

    /**
     * Register network device
     *
     * @param dev NetworkDevice pointer
     */
    void registerNetworkDevice(NetworkDevice *dev);

    /**
     * Initialize the NetworkServer.
     *
     * @return Result code
     */
    virtual FileSystem::Result initialize();

    /**
     * Called whenever another Process is terminated
     *
     * @param pid ProcessID of the terminating process
     */
    virtual void onProcessTerminated(const ProcessID pid);

    /**
     * Starts DMA on NetworkDevices after all pending requests are done
     *
     * @return True if retry is needed again, false if all requests processed
     */
    virtual bool retryRequests();

  private:

    /** Network device instance */
    NetworkDevice *m_device;
};

/**
 * @}
 * @}
 */

#endif /* __LIBNET_NETWORKSERVER_H */
