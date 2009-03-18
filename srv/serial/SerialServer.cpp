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

#include <api/IPCMessage.h>
#include <api/ProcessCtl.h>
#include <DeviceMessage.h>
#include <FileSystemMessage.h>
#include <ProcessMessage.h>
#include <Config.h>
#include <stdio.h>
#include "SerialServer.h"
#include "i8250.h"

SerialAddress SerialServer::uarts[] =
{
    { 0x3f8, 4, ZERO },
    { 0x2f8, 3, ZERO },
    { 0x3e8, 4, ZERO },
    { 0x2e8, 3, ZERO },
};
    
SerialServer::SerialServer()
    : IPCServer<SerialServer, DeviceMessage>(this)
{
    FileSystemMessage fs;
    ProcessMessage proc;
    char path[32];
    u8 lcr1, lcr2;
    bool detected = false;

    /* Register message handlers. */

    /* Attempt to detect available UART's. */
    for (Size i = 0; i < sizeof(uarts) / sizeof(SerialAddress); i++)
    {
	/* Request I/O permissions. */
        ProcessCtl(SELF, AllowIO, uarts[i].port + LINECONTROL);
    
	/* Read line control port. */
	lcr1 = inb (uarts[i].port + LINECONTROL);
	       outb(uarts[i].port + LINECONTROL, lcr1 ^ 0xff);
	       
	/* And again. */
	lcr2 = inb (uarts[i].port + LINECONTROL) ^ 0xff;
	       outb(uarts[i].port + LINECONTROL, lcr1);
	
	/* Verify we actually wrote it (means there is an UART). */
	if (lcr1 == lcr2)
	{
	    debug("detected UART at PORT=%x IRQ=%u\n",
		   uarts[i].port, uarts[i].irq);

	    uarts[i].uart = new i8250(uarts[i].port, uarts[i].irq);
	    detected = true;

	    /* Fill in path to device file. */
	    snprintf(path, sizeof(path), "/dev/serial%u", i);

	    /* Create the file. */
	    fs.createFile(path, CharacterDeviceFile, 0600,
			  proc.pid(), i);
	}
    }
    /* Did we detect at least one UART? */
    if (!detected)
    {
	proc.exit(1);
    }
}
