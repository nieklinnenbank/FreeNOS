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
#include "RecoveryMessage.h"
#include "RecoveryClient.h"

RecoveryClient::RecoveryClient(const ProcessID pid)
    : m_pid(pid)
{
}

inline Recovery::Result RecoveryClient::request(RecoveryMessage &msg) const
{
    if (ChannelClient::instance()->syncSendReceive(&msg, sizeof(msg), m_pid) == ChannelClient::Success)
    {
        return msg.result;
    }
    else
    {
        return Recovery::IpcError;
    }
}

Recovery::Result RecoveryClient::restartProcess(const ProcessID pid) const
{
    RecoveryMessage msg;
    msg.type        = ChannelMessage::Request;
    msg.action      = Recovery::RestartProcess;
    msg.pid         = pid;

    return request(msg);
}
