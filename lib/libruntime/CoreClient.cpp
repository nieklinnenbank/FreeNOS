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

#include <ChannelClient.h>
#include "CoreMessage.h"
#include "CoreClient.h"

CoreClient::CoreClient(const ProcessID pid)
    : m_pid(pid)
{
}

inline Core::Result CoreClient::request(CoreMessage &msg) const
{
    if (ChannelClient::instance()->syncSendReceive(&msg, sizeof(msg), m_pid) == ChannelClient::Success)
    {
        return msg.result;
    }
    else
    {
        return Core::IpcError;
    }
}

Core::Result CoreClient::getCoreCount(Size &coreCount) const
{
    CoreMessage msg;
    msg.type     = ChannelMessage::Request;
    msg.action   = Core::GetCoreCount;

    const Core::Result result = request(msg);
    if (result == Core::Success)
    {
        coreCount = msg.coreNumber;
    }

    return result;
}

Core::Result CoreClient::createProcess(const Size coreId,
                                       const Address programAddr,
                                       const Size programSize,
                                       const char *programCmd) const
{
    CoreMessage msg;
    msg.type        = ChannelMessage::Request;
    msg.action      = Core::CreateProcess;
    msg.coreNumber  = coreId;
    msg.programAddr = programAddr;
    msg.programSize = programSize;
    msg.programCmd  = programCmd;

    return request(msg);
}
