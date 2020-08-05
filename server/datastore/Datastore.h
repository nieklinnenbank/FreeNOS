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

#ifndef __SERVER_DATASTORE_DATASTORE_H
#define __SERVER_DATASTORE_DATASTORE_H

/**
 * @addtogroup server
 * @{
 *
 * @addtogroup datastore
 * @{
 */

namespace Datastore
{
    /**
     * Actions which may be performed on the datastore.
     */
    enum Action
    {
        RegisterBuffer = 1
    };

    /**
     * Result codes.
     */
    enum Result
    {
        Success = 0,
        IOError,
        InvalidArgument,
        IpcError
    };
}

/**
 * @}
 * @}
 */

#endif /* __SERVER_DATASTORE_DATASTORE_H */
