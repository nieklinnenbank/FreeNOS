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

#include <IPCServer.h>
#include <Types.h>
#include <Error.h>
#include "ProcessMessage.h"

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
    
    /** Process state. */
    ProcessState state;
}
UserProcess;

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
	 */
	void getIDHandler(ProcessMessage *msg);

	/**
	 * Read the user process table.
	 * @param msg Incoming message.
	 */
	void readProcessHandler(ProcessMessage *msg);

	/**
	 * Terminate a process.
	 * @param msg Incoming message.
	 */
	void exitProcessHandler(ProcessMessage *msg);

	/** User Process table. */
	static UserProcess procs[MAX_PROCS];
};

#endif /* __PROCESS_PROCESSSERVER_H */
