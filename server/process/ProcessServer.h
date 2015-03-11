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

/** 
 * @defgroup process ProcessServer (Trusted Process Server) 
 * @{   
 */

#include <IPCServer.h>
#include <Shared.h>
#include <Array.h>
#include <Types.h>
#include <Error.h>
#include "ProcessMessage.h"
#include "FileDescriptor.h"
#include "UserProcess.h"

/**
 * @brief Process management server.
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
	 * Read the user process table.
	 * @param msg Incoming message.
	 */
	void readProcessHandler(ProcessMessage *msg);

	/**
	 * Terminate a process.
	 * @param msg Incoming message.
	 */
	void exitProcessHandler(ProcessMessage *msg);
	
	/**
	 * Create a process.
	 * @param msg Incoming message.
	 */
	void spawnProcessHandler(ProcessMessage *msg);

	/**
	 * Create a copy of a process.
	 * @param msg Incoming message.
	 */
	void cloneProcessHandler(ProcessMessage *msg);

	/**
	 * Waits until a process has died.
	 * @param msg Incoming message.
	 */
	void waitProcessHandler(ProcessMessage *msg);

	/**
	 * Change the current working directory.
	 * @param msg Incoming message.
	 */
	void setCurrentDirectory(ProcessMessage *msg);

	/** User Process table. */
	Shared<UserProcess> procs;
	
	/** Per-process FileDescriptor table. */
	Array<Shared<FileDescriptor> > *files;
};

/**
 * @}
 */

#endif /* __PROCESS_PROCESSSERVER_H */
