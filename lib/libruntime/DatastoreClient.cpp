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

#include <MemoryBlock.h>
#include <ChannelClient.h>
#include <DatastoreMessage.h>
#include "DatastoreClient.h"

DatastoreClient::DatastoreClient(const ProcessID pid)
    : m_pid(pid)
{
}

Datastore::Result DatastoreClient::registerBuffer(const char *key,
                                                  void *buffer,
                                                  const Size size) const
{
    DatastoreMessage msg;
    msg.type   = ChannelMessage::Request;
    msg.action = Datastore::RegisterBuffer;
    msg.size   = size;
    MemoryBlock::copy(msg.key, key, sizeof(msg.key));

    if (ChannelClient::instance()->syncSendReceive(&msg, sizeof(msg), m_pid) == ChannelClient::Success)
    {
        *(Address *) buffer = msg.address;
        return msg.result;
    }
    else
    {
        return Datastore::IpcError;
    }
}
