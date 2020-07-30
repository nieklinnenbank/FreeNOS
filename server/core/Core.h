/*
 * Copyright (C) 2020 Niek Linnenbank
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

#ifndef __SERVER_CORE_CORE_H
#define __SERVER_CORE_CORE_H

#include <Types.h>

/**
 * @addtogroup server
 * @{
 *
 * @addtogroup core
 * @{
 */

namespace Core
{
    /**
     * Actions which may be performed on the CoreServer
     */
    enum Action
    {
        GetCoreCount = 0,
        CreateProcess,
        PingRequest,
        PongResponse
    };

    /**
     * Result code for Actions.
     */
    enum Result
    {
        Success,
        InvalidArgument,
        NotFound,
        BootError,
        ExecError,
        OutOfMemory,
        IOError,
        MemoryError,
        IpcError
    };
};

/**
 * @}
 * @}
 */

#endif /* __SERVER_CORE_CORE_H */
