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

#ifndef __TERMINAL_TERMINALSERVER_H
#define __TERMINAL_TERMINALSERVER_H

#include <api/IPCMessage.h>
#include <Types.h>
#include "VGATerminal.h"
#include "i8250Terminal.h"
#include "PS2Terminal.h"

/** Memory address of the VGA terminal to use. */
#define VGA_ADDR ((u16 *) 0x70000000)

/**
 * Actions which may be performed on an Terminal.
 */
typedef enum TerminalAction
{
    TerminalRead  = 0,
    TerminalWrite = 1,
    TerminalOK    = 2,
    TerminalError = 3,
}
TerminalAction;

/**
 * Terminal IPC message.
 */
typedef struct TerminalMessage : public Message
{
    /**
     * Default constructor.
     */
    TerminalMessage() :
	action(TerminalError), buffer(ZERO), size(ZERO)
    {
    }

    /**
     * Assignment operator.
     * @param m TerminalMessage pointer to copy from.
     */
    void operator = (TerminalMessage *m)
    {
	from   = m->from;
	type   = m->type;
	action = m->action;
	buffer = m->buffer;
	size   = m->size;
    }

    /** Action to perform. */
    TerminalAction action;
    
    /** Points to a buffer for I/O. */
    char *buffer;
    
    /** Size of the buffer. */
    Size size;
    
    /** Not used. */
    ulong unused[3];
}
TerminalMessage;

/**
 * Manages system terminals.
 */
class TerminalServer
{
    public:
    
	/**
	 * Class constructor function.
	 */
	TerminalServer();
	
	/**
	 * Enters an infinite loop, serving incoming terminal requests.
	 * @return Never.
	 */
	int run();
	
    private:
    
	/**
	 * Handles an incoming IRQ.
	 * @param msg Send by the kernel.
	 */
	void doIRQ(InterruptMessage *msg);
	
	/**
	 * Process a read request.
	 * @param msg TerminalMessage pointer.
	 */
	void doRead(TerminalMessage *msg);
	
	/**
	 * Process a write request.
	 * @param msg TerminalMessage pointer.
	 */
	void doWrite(TerminalMessage *msg);
    
	/** Video Graphics Array terminal. */
	VGATerminal vga;
	
	/** PS2 keyboard terminal. */
	PS2Terminal ps2;
	
	/** Intel 8250 UART terminal. */
	//i8250Terminal uart;
	
	/** Last read message received. */
	TerminalMessage lastRead;
	
	/** Is somebody reading? */
	bool reading;
};

#endif /* __TERMINAL_TERMINALSERVER_H */
