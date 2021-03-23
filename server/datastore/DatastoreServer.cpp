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

#include <Assert.h>
#include "DatastoreServer.h"

DatastoreServer::DatastoreServer()
    : ChannelServer<DatastoreServer, DatastoreMessage>(this)
    , m_buffers()
{
    addIPCHandler(Datastore::RegisterBuffer, &DatastoreServer::registerBuffer);
}

HashTable<String, Address> * DatastoreServer::getBufferTable(const ProcessID pid)
{
    HashTable<String, Address> * const *table = m_buffers.get(pid);

    if (table == NULL)
    {
        HashTable<String, Address> *h = new HashTable<String, Address>();

        if (h != ZERO)
            m_buffers.insert(pid, h);

        table = m_buffers.get(pid);
    }

    if (table != NULL)
    {
        return *table;
    }
    else
    {
        return ZERO;
    }
}

void DatastoreServer::registerBuffer(DatastoreMessage *msg)
{
    // Validate the buffer size
    if (msg->size > MaximumBufferSize)
    {
        ERROR("invalid buffer size: " << msg->size);
        msg->result = Datastore::InvalidArgument;
        return;
    }

    // Retrieve buffer mapping table
    HashTable<String, Address> *table = getBufferTable(msg->from);
    if (!table)
    {
        ERROR("failed to retrieve buffer mapping table for PID " << msg->from);
        msg->result = Datastore::IOError;
        return;
    }

    // Enforce NULL-terminated string for the key
    msg->key[sizeof(msg->key) - 1] = 0;

    // Setup buffer in the process
    Memory::Range range;
    range.virt = 0;
    range.phys = 0;
    range.size = msg->size;
    range.access = Memory::User | Memory::Readable | Memory::Writable;

    // Check if the key already exists
    const Address *addr = table->get(msg->key);
    if (addr != ZERO)
    {
        range.phys = *addr;
    }

    // Create mapping in the process
    const API::Result mapResult = VMCtl(msg->from, MapContiguous, &range);
    if (mapResult != API::Success)
    {
        ERROR("failed to allocate buffer `" << msg->key << "' in PID " <<
               msg->from << ": " << (int) mapResult);
        msg->result = Datastore::IOError;
        return;
    }

    // Add buffer to our administration
    if (addr == ZERO && !table->insert(msg->key, range.phys))
    {
        ERROR("failed to add buffer `" << msg->key << "' to mapping table for PID " << msg->from);
        VMCtl(msg->from, Release, &range);
        msg->result = Datastore::IOError;
        return;
    }

    // Done
    msg->address = range.virt;
    msg->result  = Datastore::Success;

    DEBUG("mapped `" << msg->key << "' for PID " << msg->from << " at " <<
           (void *) msg->address << " / " << (void *) range.phys);
}
