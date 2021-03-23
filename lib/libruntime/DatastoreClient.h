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

#ifndef __LIB_LIBRUNTIME_DATASTORECLIENT_H
#define __LIB_LIBRUNTIME_DATASTORECLIENT_H

#include <FreeNOS/API/ProcessID.h>
#include <Types.h>
#include <Memory.h>
#include <Datastore.h>

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libruntime
 * @{
 */

/**
 * Datastore client
 *
 * Provides a simple interface to the datastore server
 */
class DatastoreClient
{
  public:

    /**
     * Class constructor function.
     *
     * @param pid (Optional) Process identifier of the datastore server
     */
    DatastoreClient(const ProcessID pid = DATASTORE_PID);

    /**
     * Add a new buffer.
     *
     * If the buffer already exists, it is re-used.
     */
    Datastore::Result registerBuffer(const char *key,
                                     void *buffer,
                                     const Size size) const;

  private:

    /** Process identifier of the datastore server */
    const ProcessID m_pid;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBRUNTIME_DATASTORECLIENT_H */
