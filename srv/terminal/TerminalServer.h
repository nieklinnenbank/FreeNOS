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

#include <FileSystemMessage.h>
#include <IPCServer.h>
#include <Types.h>
#include <Version.h>
#include "Terminal.h"
#include "VGA.h"

/** Default number of terminals to support. */
#define NUM_TERMINALS 8

/** Always points to the currently active terminal. */
#define CONSOLE (NUM_TERMINALS+1)

/** Printed per default to each Terminal on creation. */
#define BANNER \
    "FreeNOS " RELEASE " (" BUILDUSER "@" BUILDHOST ") (" COMPILER ") " DATETIME "\r\n"

/**
 * Manages system terminals.
 */
class TerminalServer : public IPCServer<TerminalServer, FileSystemMessage>
{
    public:
    
	/**
	 * Class constructor function.
	 */
	TerminalServer();
	
    private:
    
	/**
	 * Handles an incoming IRQ.
	 * @param msg Send by the kernel.
	 */
	void interruptHandler(InterruptMessage *msg);
    
	/**
	 * Opens an (unused) terminal.
	 * @param msg Request message.
	 */
	void openFileHandler(FileSystemMessage *msg);

	/**
         * Reads or writes data from/to an Terminal.
         * @param msg Incoming message.
         */
        void readWriteHandler(FileSystemMessage *msg);

	/** Contains all running terminals. */
	Array<Terminal> terminals;
	
	/** Currently active terminal. */
	Terminal *current;
};

#endif /* __TERMINAL_TERMINALSERVER_H */
