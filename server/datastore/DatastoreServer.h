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

#ifndef __SERVER_DATASTORE_DATASTORESERVER_H
#define __SERVER_DATASTORE_DATASTORESERVER_H

#include <FreeNOS/ProcessManager.h>
#include <ChannelServer.h>
#include <Types.h>
#include <HashTable.h>
#include <String.h>
#include "Datastore.h"
#include "DatastoreMessage.h"

/**
 * @addtogroup server
 * @{
 *
 * @addtogroup datastore
 * @{
 */

/**
 * Datastore Server
 *
 * Provides a key/value in-memory based data storage that can be used system wide.
 */
class DatastoreServer : public ChannelServer<DatastoreServer, DatastoreMessage>
{
  private:

    /** Maximum size of a single buffer */
    static const Size MaximumBufferSize = KiloByte(256);

  public:

    /**
     * Class constructor function.
     */
    DatastoreServer();

  private:

    /**
     * Retrieve current buffer table for given ProcessID.
     *
     * @param pid Process identifier
     *
     * @return Pointer to a HashTable on success, NULL on failure.
     */
    HashTable<String, Address> * getBufferTable(const ProcessID pid);

    /**
     * Add a new buffer.
     *
     * @param msg DatastoreMessage pointer
     */
    void registerBuffer(DatastoreMessage *msg);

  private:

    /** Per-process hash table with key to buffers mapping. */
    HashTable<ProcessID, HashTable<String, Address> *> m_buffers;
};

/**
 * @}
 * @}
 */

#endif /* __SERVER_DATASTORE_DATASTORESERVER_H */
