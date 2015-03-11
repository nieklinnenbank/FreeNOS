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

#include <API/ProcessCtl.h>
#include <DeviceServer.h>
#include <MemoryMessage.h>
#include <ProcessID.h>
#include "UHCIController.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

UHCIController::UHCIController(const char *bus,
			       const char *slot,
			       const char *func)
    : PCIDevice(bus, slot, func)
{
}

Error UHCIController::initialize()
{
    /* Open the system log. */
    openlog("USB", LOG_PID | LOG_CONS, LOG_USER);
    
    /* Print out UHCI Controller information. */
    syslog(LOG_INFO, "UHCI Host Controller at IOADDR=%x IRQ=%x",
	   readLong(PCI_BAR4), readByte(PCI_IRQ));
    
    return ESUCCESS;
}

Error UHCIController::interrupt(Size vector)
{
    syslog(LOG_INFO, "UHCI Interrupt on IRQ %u", vector);
    return ESUCCESS;
}
