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
#include <Vector.h>
#include <Init.h>
#include "Init.h"

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
    Map    	= 2,
    Send   	= 3,
    Receive 	= 4,
    SendReceive = 5,
    Read 	= 6,
    Write 	= 7,
    Lookup      = 8,
}
Action;

/** List of known APIHandler functions. */
extern Vector<APIHandler> apis;

#endif /* __KERNEL_API_H */
