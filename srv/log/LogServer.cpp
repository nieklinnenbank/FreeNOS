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

#include <API/VMCopy.h>
#include <stdio.h>
#include <fcntl.h>
#include "LogServer.h"
#include "LogMessage.h"

LogServer::LogServer() : IPCServer<LogServer, LogMessage>(this)
{
    /* Open standard I/O. */
    for (int i = 0; i < 3; i++)
    {
	while (open("/dev/console", ZERO) == -1);
    }
    /* Register message handlers. */
    addIPCHandler(WriteLog, &LogServer::writeLogHandler, false);
}

void LogServer::writeLogHandler(LogMessage *msg)
{
    /* Write to the system console. */
    printf("PID[%d]: %s\r\n", msg->from, msg->buffer);
}
