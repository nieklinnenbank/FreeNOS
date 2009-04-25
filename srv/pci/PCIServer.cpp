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

#include <api/ProcessCtl.h>
#include <LogMessage.h>
#include <stdlib.h>
#include "PCIServer.h"
    
PCIServer::PCIServer()
    : IPCServer<PCIServer, PCIMessage>(this)
{
    PCIDevice dev;

    /* Request I/O ports. */
    for (int i = 0; i < 4; i++)
    {
	ProcessCtl(SELF, AllowIO, PCI_CONFADDR + i);
	ProcessCtl(SELF, AllowIO, PCI_CONFDATA + i);
    }
    /* Register message handlers. */
    addIPCHandler(GetDevice, &PCIServer::getDeviceHandler);
    
    /* Detect PCI host controller. */
    if (PCI_READ_WORD(0, 0, 0, PCI_VID) == 0xffff ||
        PCI_READ_WORD(0, 0, 0, PCI_DID) == 0xffff)
    {
	log("PCI: no host controller found");
	exit(1);
    }
    else
	log("PCI: host controller found");
    
    /* Scan the PCI bus. */
    for (u16 bus = 0; bus < 0xff; bus++)
    {
	for (u16 slot = 0; slot < 0x8; slot++)
        {
	    /* Read device ID's. */
    	    dev.vendorID = PCI_READ_WORD(bus, slot, 0, PCI_VID);
    	    dev.deviceID = PCI_READ_WORD(bus, slot, 0, PCI_DID);

	    /* Is this a valid device? */
    	    if (dev.vendorID != 0xffff &&
	        dev.deviceID != 0xffff)
    	    {
		/* Read out configuration. */
		dev.revisionID = PCI_READ_BYTE(bus, slot, 0, PCI_RID);
		dev.irq        = PCI_READ_BYTE(bus, slot, 0, PCI_IRQ);
		dev.bar0       = (u8 *) PCI_READ_LONG(bus, slot, 0, PCI_BAR0);
		dev.bar1       = (u8 *) PCI_READ_LONG(bus, slot, 0, PCI_BAR1);
		dev.bar2       = (u8 *) PCI_READ_LONG(bus, slot, 0, PCI_BAR2);
		dev.bar3       = (u8 *) PCI_READ_LONG(bus, slot, 0, PCI_BAR3);
		dev.bar4       = (u8 *) PCI_READ_LONG(bus, slot, 0, PCI_BAR4);
		dev.bar5       = (u8 *) PCI_READ_LONG(bus, slot, 0, PCI_BAR5);
		
		/* Append to list of devices. */
		devices.insertTail(new PCIDevice(&dev));

		/* Log the device. */
	        log("[%x:%x] %x:%x (rev %d)",
        	     bus, slot,    dev.vendorID,
		     dev.deviceID, dev.revisionID);
    	    }
	}
    }
}

void PCIServer::getDeviceHandler(PCIMessage *msg)
{
    msg->result = ENOSUCH;

    /* Lookup the device. */
    for (ListIterator<PCIDevice> i(&devices); i.hasNext(); i++)
    {
	if (i.current()->vendorID == msg->dev.vendorID &&
	    i.current()->deviceID == msg->dev.deviceID)
	{
	    msg->dev     = *i.current();
	    msg->result = ESUCCESS;
	    break;
	}
    }
}
