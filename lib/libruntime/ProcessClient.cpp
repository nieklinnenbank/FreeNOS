/*
 * Copyright (C) 2015 Niek Linnenbank
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

#include <FreeNOS/System.h>
#include <Types.h>
#include <Macros.h>
#include "ProcessClient.h"

const ProcessID ProcessClient::m_pid = ProcessCtl(SELF, GetPID, 0);

const ProcessID ProcessClient::m_parent = ProcessCtl(SELF, GetParent, 0);

ProcessID ProcessClient::getProcessID() const
{
    return m_pid;
}

ProcessID ProcessClient::getParentID() const
{
    return m_parent;
}

ProcessClient::Result ProcessClient::processInfo(const ProcessID pid,
                                                 ProcessClient::Info &info) const
{
#ifndef __HOST__
    const char * textStates[] = {
        "Ready",
        "Sleeping",
        "Waiting",
        "Stopped"
    };
    const Arch::MemoryMap map;
    const Memory::Range range = map.range(MemoryMap::UserArgs);
    char cmd[128];

    const API::Result result = ProcessCtl(pid, InfoPID, (Address) &info.kernelState);
    switch (result)
    {
        case API::Success:
            break;
        case API::NotFound:
            return NotFound;
        default:
            return IOError;
    }

    // Read the full command
    if (VMCopy(pid, API::Read, (Address) cmd, range.virt, sizeof(cmd)) != API::Success)
    {
        return IOError;
    }

    // Fill output
    info.command = cmd;
    info.textState = (pid == m_pid ? "Running" : textStates[info.kernelState.state]);
#endif /* __HOST__ */

    return Success;
}

ProcessClient::Result ProcessClient::processInfo(const String program,
                                                 ProcessClient::Info &info) const
{
    // Loop processes
    for (ProcessID i = 0; i < MaximumProcesses; i++)
    {
        const Result result = processInfo(i, info);
        if (result == Success && info.command.equals(program))
        {
            return result;
        }
    }

    return NotFound;
}

ProcessID ProcessClient::findProcess(const String program) const
{
    ProcessClient::Info info;

    const Result result = processInfo(program, info);
    if (result == Success)
    {
        return info.kernelState.id;
    }
    else
    {
        return ANY;
    }
}
