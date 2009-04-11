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

#ifndef __LOG_LOGMESSAGE_H
#define __LOG_LOGMESSAGE_H

#include <api/IPCMessage.h>
#include <Types.h>
#include <Macros.h>
#include <Error.h>
#include <Config.h>

/**
 * Output a log message to the log server.
 * @param fmt Formatted message.
 * @param ... Argument list.
 */
#define log(fmt, ...) \
    ({ \
	LogMessage _msg; \
	\
	_msg.size   = snprintf(_msg.buffer, sizeof(_msg.buffer), \
			        fmt, ##__VA_ARGS__); \
	_msg.action = WriteLog; \
	_msg.ipc(LOGSRV_PID, Send, sizeof(_msg)); \
    })
    
/**
 * Logs a debug message to the log server.
 * @param fmt Formatted message.
 * @param ... Argument list.
 */
#define debug(fmt, ...) \
    ({ \
	LogMessage _msg; \
	char *_file = __FILE__; \
	char *_top  = IQUOTE(__TOP__); \
	\
	while (*_file == *_top) \
	{ \
	    _file++, _top++; \
	} \
	_msg.size   = snprintf(_msg.buffer, sizeof(_msg.buffer), \
			       "{%s:%d}: " fmt, \
			       _file, __LINE__, ##__VA_ARGS__); \
	_msg.action = WriteLog; \
	_msg.ipc(LOGSRV_PID, Send, sizeof(_msg)); \
    })

/**
 * Actions which can be specified in an LogMessage.
 */
typedef enum LogAction
{
    WriteLog = 0,
    ReadLog  = 1,
}
LogAction;

/**
 * Log operation message.
 */
typedef struct LogMessage : public Message
{
    /**
     * Default constructor.
     */
    LogMessage() : action(WriteLog)
    {
    }

    union
    {
	/** Action to perform. */
        LogAction action;
	
	/** Result code. */
	Error result;
    };

    /** Logging I/O buffer. */
    char buffer[32];
    
    /** Size of the buffer. */
    Size size;
}
LogMessage;

#endif /* __LOG_LOGMESSAGE_H */
