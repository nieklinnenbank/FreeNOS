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

#ifndef __PROCESS_PROCESSSERVER_H
#define __PROCESS_PROCESSSERVER_H

#include <api/IPCMessage.h>
#include <IPCServer.h>
#include <Types.h>

/**
 * Actions which can be specified in an ProcessMessage.
 */
typedef enum ProcessAction
{
    GetID     = 0,
    ProcessOK = 1,
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
    ProcessMessage() : action(GetID), number(ZERO)
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

    /** Unused. */
    ulong unused[3];
}
ProcessMessage;

/**
 * Process management server.
 */
class ProcessServer : public IPCServer<ProcessServer, ProcessMessage>
{
    public:
    
	/**
	 * Class constructor function.
	 */
	ProcessServer();

    private:
    
	/**
	 * Retrieves the PID of the caller.
	 */
	void doGetID(ProcessMessage *msg, ProcessMessage *reply);
};

#endif /* __PROCESS_PROCESSSERVER_H */
