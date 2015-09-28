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

#ifndef __SERVER_CORE_COREMESSAGE_H
#define __SERVER_CORE_COREMESSAGE_H

/** 
 * @defgroup core CoreServer
 * @{   
 */

#include <FreeNOS/API.h>
#include <Types.h>
#include <Macros.h>
#include <Memory.h>

/** @see UserProcess.h */
struct UserProcess;

/** @see FileSystemMount.h */
struct FileSystemMount;

/**
 * Actions which can be specified in a CoreMessage
 */
typedef enum CoreAction
{
    Ping = 1,
    Pong,
    CreateProcess,
    GetCoreCount
}
CoreAction;

/**
 * Core operation message.
 */
typedef struct CoreMessage : public Message
{
    union
    {
        /** Action to perform. */
        CoreAction action;

        /** Result code. */
        Error result;
    };

    uint coreId;

    /** Physical address of the program image to start. */
    Address program;

    /** Size of the program image. */
    Size programSize;

    /** Program arguments */
    char *programCommand;

    union
    {
        // TODO: remove. it is used by the libposix Runtime hack for file descriptors
        char *path;
        Size coreCount;
    };
}
CoreMessage;

/**
 * @}
 */

#endif /* __SERVER_CORE_COREMESSAGE_H */
