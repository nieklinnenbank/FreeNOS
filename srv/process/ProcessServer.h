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
#include <api/SystemInfo.h>
#include <api/VMCopy.h>
#include <arch/Memory.h>
#include <IPCServer.h>
#include <FileSystemMessage.h>
#include <Types.h>
#include <Error.h>

/** Maximum length of a command (as saved in the user process table). */
#define COMMANDLEN 64

/**
 * Userlevel process information.
 */
typedef struct UserProcess
{
    /** Command string. */
    char command[COMMANDLEN];

    /** User and Group ID. */
    u16 uid, gid;
}
UserProcess;

/**
 * Actions which can be specified in an ProcessMessage.
 */
typedef enum ProcessAction
{
    GetID       = 0,
    ReadProcess = 1,
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
	 * @param msg Incoming message.
	 * @param reply Response message.
	 */
	void getIDHandler(ProcessMessage *msg, ProcessMessage *reply);

	/**
	 * Read the user process table.
	 * @param msg Incoming message.
	 * @param reply Response message.
	 */
	void readProcessHandler(ProcessMessage *msg, ProcessMessage *reply);

	/** User Process table. */
	static UserProcess procs[MAX_PROCS];
};

#endif /* __PROCESS_PROCESSSERVER_H */
