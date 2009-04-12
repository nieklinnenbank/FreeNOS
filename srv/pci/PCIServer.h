/*
 * Copyright (C) 2009 Niek Linnenbank
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

#ifndef __PCI_PCISERVER_H
#define __PCI_PCISERVER_H

#include <IPCServer.h>
#include <Types.h>
#include <Error.h>
#include <List.h>
#include "PCIDevice.h"
#include "PCIMessage.h"

/**
 * PCI host controller server.
 */
class PCIServer : public IPCServer<PCIServer, PCIMessage>
{
    public:
    
	/**
	 * Class constructor function.
	 */
	PCIServer();

    private:

	/**
	 * Retrieve PCI device information.
	 * @param msg Request message.
	 */
	void getDeviceHandler(PCIMessage *msg);

	/** Contains all detected devices. */
	List<PCIDevice> devices;
};

#endif /* __PCI_PCISERVER_H */
