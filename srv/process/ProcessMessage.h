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

#ifndef __PROCESS_PROCESSMESSAGE_H
#define __PROCESS_PROCESSMESSAGE_H

#include <API/IPCMessage.h>
#include <Types.h>
#include <Macros.h>
#include <Config.h>

/** @see ProcessServer.h */
class UserProcess;

/**
 * Actions which can be specified in an ProcessMessage.
 */
typedef enum ProcessAction
{
    GetID        = 0,
    ReadProcess  = 1,
    ExitProcess  = 2,
    SpawnProcess = 3,
    WaitProcess  = 4
}
ProcessAction;

/**
 * Process operation message.
 */
typedef struct ProcessMessage : public Message
{
    /**
     * Default constructor.
     */
    ProcessMessage() : action(GetID), number(ZERO), buffer(ZERO)
    {
    }

    /**
     * Assignment operator.
     * @param m ProcessMessage pointer to copy from.
     */
    void operator = (ProcessMessage *m)
    {
	from    = m->from;
	type    = m->type;
	action  = m->action;
	number  = m->number;
	buffer  = m->buffer;
    }

    /**
     * Terminate ourselves.
     * @param status Exit code.
     * @return Never.
     */
    void exit(int status)
    {
	number = status;
	action = ExitProcess;
	ipc(PROCSRV_PID, SendReceive, sizeof(*this));
    }

    /**
     * Retrieve our process ID number (PID).
     * @return Process ID number.
     */
    ProcessID pid()
    {
	action = GetID;
	ipc(PROCSRV_PID, SendReceive, sizeof(*this));
	return number;
    }

    union
    {
	/** Action to perform. */
	ProcessAction action;
	
	/** Result code. */
	Error result;
    };

    /** Used to store somekind of number (e.g. PID's). */
    ulong number;
    
    /** Input/Output buffer for ReadProcess. */
    UserProcess *buffer;

    /** Path to an executable program. */
    char *path;

    /** Unused. */
    ulong unused[2];
}
ProcessMessage;

#endif /* __PROCESS_PROCESSMESSAGE_H */
