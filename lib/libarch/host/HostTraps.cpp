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

#include <FreeNOS/User.h>
#include <FreeNOS/Config.h>
#include <unistd.h>
#include "HostTraps.h"

static API::Result hostProcessCtlHandler(ProcessID procID,
                                         ProcessOperation action,
                                         Address addr,
                                         Address output)
{
    switch (action)
    {
        case GetPID:
            return (API::Result) getpid();

        default:
            break;
    }

    return API::InvalidArgument;
}

static API::Result hostSystemInfoHandler(SystemInformation *info)
{
    // Clear the structure and fill strings
    MemoryBlock::set(info, 0, sizeof(*info));
    MemoryBlock::copy(info->cmdline, (char *) "kernel", sizeof(info->cmdline));

    // Set various members
    info->version     = VERSIONCODE;
    info->memorySize  = MegaByte(256);
    info->memoryAvail = MegaByte(128);
    info->coreId      = 0;

    return API::Success;
}

static API::Result hostVMShareHandler(ProcessID procID,
                                      API::Operation op,
                                      ProcessShares::MemoryShare *share)
{
    return API::Success;
}

static API::Result hostApiHandler(ulong api, ulong arg1, ulong arg2, ulong arg3, ulong arg4, ulong arg5)
{
    switch (api)
    {
        case API::ProcessCtlNumber:
            return hostProcessCtlHandler(arg1, (ProcessOperation) arg2, arg3, arg4);

        case API::SystemInfoNumber:
            return hostSystemInfoHandler((SystemInformation *) arg1);

        case API::VMShareNumber:
            return hostVMShareHandler(arg1, (API::Operation) arg2, (ProcessShares::MemoryShare *) arg3);

        default:
            break;
    }

    return API::InvalidArgument;
}

ulong trapKernel1(ulong num, ulong arg1)
{
    return hostApiHandler(num, arg1, 0, 0, 0, 0);
}

ulong trapKernel2(ulong num, ulong arg1, ulong arg2)
{
    return hostApiHandler(num, arg1, arg2, 0, 0, 0);
}

ulong trapKernel3(ulong num, ulong arg1, ulong arg2, ulong arg3)
{
    return hostApiHandler(num, arg1, arg2, arg3, 0, 0);
}

ulong trapKernel4(ulong num, ulong arg1, ulong arg2, ulong arg3, ulong arg4)
{
    return hostApiHandler(num, arg1, arg2, arg3, arg4, 0);
}

ulong trapKernel5(ulong num, ulong arg1, ulong arg2, ulong arg3, ulong arg4, ulong arg5)
{
    return hostApiHandler(num, arg1, arg2, arg3, arg4, arg5);
}
