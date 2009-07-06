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

#include <API/IPCMessage.h>
#include <API/ProcessCtl.h>
#include <FileSystemMessage.h>
#include <FileMode.h>
#include <ProcessMessage.h>
#include <LogMessage.h>
#include <Config.h>
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
    : IPCServer<SerialServer, FileSystemMessage>(this)
{
    FileSystemMessage fs;
    ProcessMessage proc;
    char path[32];
    u8 lcr1, lcr2;
    bool detected = false;

    /* Register message handlers. */
    addIPCHandler(OpenFile,  &SerialServer::openHandler, true);
    addIPCHandler(ReadFile,  &SerialServer::readWriteHandler, false);
    addIPCHandler(WriteFile, &SerialServer::readWriteHandler, false);

    /* Attempt to detect available UART's. */
    for (Size i = 0; i < MAX_UARTS; i++)
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
	    /* Create new instance. */
	    uarts[i].dev = new i8250(uarts[i].port, uarts[i].irq);
	    detected = true;

	    /* Fill in path to device file. */
	    snprintf(path, sizeof(path), "/dev/serial%u", i);

	    /* Create the file. */
	    fs.createFile(path, CharacterDeviceFile, OwnerRW,
			  proc.pid(), i);
			  
	    /* Register IRQ handler. */
	    addIRQHandler(uarts[i].irq, &SerialServer::interruptHandler);
	    
	    /* Perform log. */
	    log("i8250 at PORT=%x IRQ=%x",
	         uarts[i].port, uarts[i].irq);
	}
    }
    /* Did we detect at least one UART? */
    if (!detected)
    {
	proc.exit(1);
    }
}

void SerialServer::openHandler(FileSystemMessage *msg)
{
    msg->action = IODone;
    msg->result = ESUCCESS;
}

void SerialServer::readWriteHandler(FileSystemMessage *msg)
{
    SerialDevice *dev;
    Error e;

    /* Is the given UART available? */
    if (msg->deviceID.minor < MAX_UARTS &&     
       (dev = uarts[msg->deviceID.minor].dev) &&
       (!dev->isRequestPending()))
    {
	/* Attempt to perform I/O. */
	switch (msg->action)
	{
	    case ReadFile:
		    dev->flush();
		e = dev->bufferedRead(msg);
		break;
	
	    case WriteFile:
		e = dev->bufferedWrite(msg);
		    dev->flush();
		break;

	    default:
		e = ENOTSUP;
	}
	/* Did it fail? */
	if (e < 0)
	{
	    msg->result = e;
	}
	/* Is data pending? */
	else if (e == 0)
	{
	    return;
	}
	/* Success. */
	else
	{
	    msg->size   = e;
	    msg->result = ESUCCESS;
	}
    }
    else
	msg->result = EACCES;

    /* Send reply. */
    msg->action = IODone;
    msg->ipc(msg->from, Send, sizeof(*msg));
}

void SerialServer::interruptHandler(InterruptMessage *msg)
{
    for (Size i = 0; i < MAX_UARTS; i++)
    {
	if (uarts[i].irq == msg->vector &&
	    uarts[i].dev->flush())
	{
	    break;
	}
    }
}
