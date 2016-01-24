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

#ifndef __API_PRIVEXEC_H
#define __API_PRIVEXEC_H

#include <FreeNOS/System.h>

/**  
 * @defgroup kernelapi kernel (API) 
 * @{  
 */

/**
 * Available operations to perform using PrivExec().
 * @see PrivExec
 */
typedef enum PrivOperation
{
    Idle         = 0,
    Reboot       = 1,
    Shutdown     = 2,
    WriteConsole = 3
}
PrivOperation;

/**
 * Prototype for user applications. Performs various privileged operations.
 * @param op The operation to perform.
 * @return Zero on success and error code on failure.
 */
inline Error PrivExec(PrivOperation op, Address param = 0)
{
    return trapKernel2(API::PrivExecNumber, op, param);
}

/**
 * Prototype for kernel handler.
 */
extern Error PrivExecHandler(PrivOperation op, Address param);

/**
 * @}
 */

#endif /* __API_PRIVEXEC_H */
