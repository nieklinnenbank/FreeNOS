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

#include <DeviceServer.h>
#include "IDEController.h"
#include <Error.h>
#include <Types.h>
#include <stdlib.h>
#include <syslog.h>

int main(int argc, char **argv)
{
    DeviceServer server("ide", CharacterDeviceFile);

    /*
     * Verify command-line arguments.
     */
    if (argc < 4)
    {
	return EXIT_SUCCESS;
    }
    /*
     * Start serving requests.
     */
    server.add(new IDEController(argv[1], argv[2], argv[3]));
    return server.run();
}

IDEController::IDEController(const char *bus,
			     const char *slot,
			     const char *func)
    : PCIDevice(bus, slot, func)
{
}

Error IDEController::initialize()
{
    /* Open the system log. */
    openlog("IDE", LOG_PID | LOG_CONS, LOG_USER);
    
    /* Print out IDE Controller information. */
    syslog(LOG_INFO, "IDE Host Controller at IOADDR=%x IRQ=%x",
	   readLong(PCI_BAR4), readByte(PCI_IRQ));
    
    return ESUCCESS;
}

Error IDEController::interrupt(Size vector)
{
    syslog(LOG_INFO, "UHCI Interrupt on IRQ %u", vector);
    return ESUCCESS;
}
