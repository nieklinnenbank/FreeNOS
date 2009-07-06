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
#include <API/VMCopy.h>
#include <API/VMCtl.h>
#include <API/ProcessCtl.h>
#include <Types.h>
#include <Version.h>
#include <IPCServer.h>
#include <FileSystem.h>
#include <FileSystemMessage.h>
#include <FileMode.h>
#include <ProcessMessage.h>
#include "TerminalServer.h"
#include "Terminal.h"
#include "Keyboard.h"

TerminalServer::TerminalServer()
    : IPCServer<TerminalServer, FileSystemMessage>(this), current(ZERO)
{
    FileSystemMessage fs;
    ProcessMessage proc;
    Terminal *term;
    char path[PATHLEN];

    /* Register IPC handlers. */
    addIPCHandler(OpenFile,  &TerminalServer::openFileHandler,  true);
    addIPCHandler(ReadFile,  &TerminalServer::readWriteHandler, false);
    addIPCHandler(WriteFile, &TerminalServer::readWriteHandler, false);
    addIRQHandler(PS2_IRQ,   &TerminalServer::interruptHandler);

    /* Request VGA memory. */
    VMCtl(Map, SELF, VGA_PADDR, VGA_VADDR,
	  PAGE_PRESENT | PAGE_USER | PAGE_RW | PAGE_PINNED);

    /* Request PS2 keyboard I/O port and IRQ. */
    ProcessCtl(SELF, AllowIO,  PS2_PORT);
    ProcessCtl(SELF, WatchIRQ, PS2_IRQ);

    /* Create terminals. */
    for (int i = 0; i < NUM_TERMINALS; i++)
    {
	/* Fill in correct path. */
	snprintf(path, sizeof(path), "/dev/tty%d", i);
	
	/* Create device file. */
	fs.createFile(path, CharacterDeviceFile, OwnerRW, proc.pid(), i);
	
	/* Allocate a Terminal. */
	term = new Terminal;
	terminals.insert(i, term);
	
	/* Print banner and copyright notice. */
	term->write(BANNER,    strlen(BANNER));
	term->write(COPYRIGHT, strlen(COPYRIGHT));
    }
    /* Points to the currently active console. */
    snprintf(path, sizeof(path), "/dev/console");
    fs.createFile(path, CharacterDeviceFile, OwnerW, proc.pid(), CONSOLE);
    
    /* Activate current console. */
    current = terminals[0];
    current->activate();
}

void TerminalServer::openFileHandler(FileSystemMessage *msg)
{
    // TODO: Check that the given terminal isn't already opened.
    msg->action = IODone;
    msg->result = ESUCCESS;

#if 0
    /* Check that the given terminal isn't already opened. */
    if (msg->deviceID.minor < NUM_TERMINALS &&
        !terminals[msg->deviceID.minor])
    {
    }
    /* Or are we opening the currently active console? */
    else if (msg->deviceID.minor == CONSOLE)
	reply->result = ESUCCESS;
	
    /* Request cannot be allowed. */
    else
	reply->result = EACCES;
#endif
}

void TerminalServer::readWriteHandler(FileSystemMessage *msg)
{
    char buffer[128];

    /* Calculate the number of bytes to copy. */
    Size num = msg->size < sizeof(buffer) ?
               msg->size : sizeof(buffer);

    switch (msg->action)
    {
	case WriteFile:
	
	    /* Write to the VGA terminal. */
	    if ((num = VMCopy(msg->procID, Read, (Address) buffer,
			     (Address) msg->buffer, num)) > 0)
	    {
		current->write(buffer, num);
		msg->ioDone(msg->from, msg->procID,
			    num, ESUCCESS);
	    }
	    break;

	case ReadFile:
	
	    /* Read from the keyboard buffer. */
	    if ((num = current->read(buffer, num)) > 0)
	    {
		VMCopy(msg->procID, Write, (Address) buffer,
		      (Address) msg->buffer, num);
		current->setRequest(ZERO);
		msg->ioDone(msg->from, msg->procID, num, ESUCCESS);
	    }
	    else
		current->setRequest(msg);
	    break;

	default:
	    ;
    }
}

void TerminalServer::interruptHandler(InterruptMessage *msg)
{
    /* Attempt to read/write more. */
    if (current->flush() && current->getRequest())
    {
	/* Further process the currently active request. */
	readWriteHandler(current->getRequest());
    }
}
