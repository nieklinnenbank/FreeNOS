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

#ifndef __LOG_LOGSERVER_H
#define __LOG_LOGSERVER_H

#include <api/IPCMessage.h>
#include <IPCServer.h>
#include <Types.h>
#include <Macros.h>
#include <Error.h>
#include "LogMessage.h"

/**
 * Logs output and debug messages.
 */
class LogServer : public IPCServer<LogServer, LogMessage>
{
    public:
    
	/**
	 * Class constructor function.
	 */
	LogServer();

    private:
    
	/**
	 * Write a message to the log.
	 * @param msg Request message.
	 */
	void writeLogHandler(LogMessage *msg);

};

#endif /* __LOG_LOGSERVER_H */
