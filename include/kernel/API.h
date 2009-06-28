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

#ifndef __KERNEL_API_H
#define __KERNEL_API_H

#include <Types.h>
#include <Array.h>
/* #include <Array.h> */
#include <Init.h>
#include "Init.h"

/**
 * @defgroup kernel kernel (generic)
 * @{
 */

/**
 * Initializes an APIHandler.
 * @param nr Unique system call number.
 * @param handler APIHandler function.
 */
#define INITAPI(nr,handler) \
    void __api_##nr##handler () \
    { \
	apis.insert(nr, (APIHandler *)handler); \
    } \
    INITFUNC(__api_##nr##handler, API)

/**
 * Function which handles an kernel API (system call) request.
 * @return Status code of the APIHandler execution.
 */
typedef ulong APIHandler(ulong, ulong, ulong, ulong, ulong);

/**
 * Various actions which may be performed inside an APIHandler.
 */
typedef enum Operation
{
    Create 	= 0,
    Delete 	= 1,
    Send   	= 2,
    Receive 	= 3,
    SendReceive = 4,
    Read 	= 5,
    Write 	= 6,
}
Operation;

/** List of known APIHandler functions. */
extern Array<APIHandler> apis;

/**
 * @}
 */

#endif /* __KERNEL_API_H */
