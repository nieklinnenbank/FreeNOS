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

#ifndef __KERNEL_API_PROCESSCTL_H
#define __KERNEL_API_PROCESSCTL_H

#include <FreeNOS/Process.h>
#include <Types.h>

/**
 * @addtogroup kernel
 * @{
 *
 * @addtogroup kernelapi
 * @{
 */

/**
 * Available operation to perform using ProcessCtl.
 *
 * @see ProcessCtl
 */
typedef enum ProcessOperation
{
    Spawn = 0,
    KillPID,
    GetPID,
    GetParent,
    WatchIRQ,
    EnableIRQ,
    DisableIRQ,
    SendIRQ,
    InfoPID,
    WaitPID,
    InfoTimer,
    WaitTimer,
    EnterSleep,
    Schedule,
    Wakeup,
    Stop,
    Resume,
    Reset
}
ProcessOperation;

/**
 * Process information structure, used for Info.
 */
typedef struct ProcessInfo
{
    /** Process Identity number. Must be unique. */
    ProcessID id;

    /** Parent process id. */
    ProcessID parent;

    /** Defines the current state of the Process. */
    Process::State state;
}
ProcessInfo;

/** Operator to print a ProcessOperation to a Log */
Log & operator << (Log &log, ProcessOperation op);

/**
 * Prototype for user applications. Process management related operations.
 *
 * @param proc Target Process' ID.
 * @param op The operation to perform.
 * @param addr Input argument address, used for program entry point for Spawn,
 *             ProcessInfo pointer for Info.
 * @param output Output argument address (optional).
 *
 * @return API::Success on success and other API::ErrorCode on failure.
 *         For WaitPID, the process exit status is stored in the upper 16-bits
 *         of this return value on success. For Spawn, the new PID is stored in
 *         the upper 16-bits.
 */
inline API::Result ProcessCtl(const ProcessID proc,
                              const ProcessOperation op,
                              const Address addr = 0,
                              const Address output = 0)
{
    return (API::Result) trapKernel4(API::ProcessCtlNumber, proc, op, addr, output);
}

/**
 * @}
 */

#ifdef __KERNEL__

/**
 * @addtogroup kernelapi_handler
 * @{
 */

/**
 * Kernel handler prototype. Process management related operations.
 *
 * @param proc Target Process' ID.
 * @param op The operation to perform.
 * @param addr Input argument address, used for program entry point for Spawn,
 *             ProcessInfo pointer for Info.
 * @param output Output argument address (optional).
 *
 * @return API::Success on success and other API::ErrorCode on failure.
 *         For WaitPID, the process exit status is stored in the upper 16-bits
 *         of this return value on success. For Spawn, the new PID is stored in
 *         the upper 16-bits.
 */
extern API::Result ProcessCtlHandler(const ProcessID proc,
                                     const ProcessOperation op,
                                     const Address addr,
                                     const Address output);

/**
 * @}
 */

#endif /* __KERNEL__ */

/**
 * @}
 */

#endif /* __KERNEL_API_PROCESSCTL_H */
