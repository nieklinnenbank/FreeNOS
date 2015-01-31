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

#ifndef __API_PROCESSCTL_H
#define __API_PROCESSCTL_H

#include <FreeNOS/API.h>
#include <FreeNOS/Scheduler.h>
#include <Error.h>
#include <Types.h>

/**  
 * @defgroup kernelapi kernel (API) 
 * @{  
 */

/** SystemCall number for ProcessCtl(). */
#define PROCESSCTL 4

/**
 * Available operation to perform using PrivExec().
 * @see PrivExec
 */
typedef enum ProcessOperation
{
    Spawn    = 0,
    KillPID  = 1,
    GetPID   = 2,
    AllowIO  = 3,
    WatchIRQ = 4,
    InfoPID  = 5,
    Schedule = 6,
    Resume   = 7,
    SetStack = 8,
}
ProcessOperation;

/**
 * Process information structure, used for Info.
 */
typedef struct ProcessInfo
{
    /** Process Identity number. Must be unique. */
    ProcessID id;
    
    /** Defines the current state of the Process. */
    ProcessState state;
    
    /** Virtual address of the stack. */
    Address stack;
    
    /** Physical address of the page directory. */
    Address pageDirectory;
}
ProcessInfo;

/**
 * Prototype for user applications. Process management related operations.
 * @param proc Target Process' ID.
 * @param op The operation to perform.
 * @param addr Argument address, used for program entry point for Spawn,
 *             ProcessInfo pointer for Info.
 * @return Zero on success and error code on failure.
 */
inline Error ProcessCtl(ProcessID proc, ProcessOperation op, Address addr = 0)
{
    return trapKernel3(PROCESSCTL, proc, op, addr);
}

/**
 * @}
 */

#endif /* __API_PROCESSCTL_H */
