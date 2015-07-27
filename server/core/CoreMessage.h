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
    /* FileSystem mounts. */
    GetMounts,
    SetMount,
    
    /* Diagnostics. */
    ProcessMemory
}
CoreAction;

/**
 * Core operation message.
 */
// TODO: remove the UGLY ::Range inheritance...
typedef struct CoreMessage : public Message, public Memory::Range
{
    union
    {
        /** Action to perform. */
        CoreAction action;

        /** Result code. */
        Error result;
    };
    
    /** Used to store somekind of number (e.g. PID's). */
    ulong number;

    /** FileSystemMounts table */
    FileSystemMount *mounts;
    
    /** Path to an executable program, or FileSystemMount path. */
    char *path;
}
CoreMessage;

/**
 * @}
 */

#endif /* __SERVER_CORE_COREMESSAGE_H */
