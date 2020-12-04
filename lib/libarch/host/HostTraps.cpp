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
#include <Log.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "HostShares.h"
#include "HostTraps.h"

static API::Result hostPrivExecHandler(PrivOperation op, Address addr)
{
    switch (op)
    {
        case RebootSystem:
            ::exit(1);
            break;

        case ShutdownSystem:
            ::exit(2);
            break;

        case WriteConsole:
            fprintf(stdout, "%s", (char *) addr);
            fflush(stdout);
            break;

        case Panic:
            fprintf(stderr, "Panic\n");
            fflush(stderr);
            ::exit(3);
            break;

        default:
            break;
    }

    return API::InvalidArgument;
}

static API::Result hostProcessCtlHandler(ProcessID procID,
                                         ProcessOperation action,
                                         Address addr,
                                         Address output)
{
    switch (action)
    {
        case GetPID:
            return (API::Result) getpid();

        case EnterSleep:
        {
            usleep(100000);
            return API::Success;
        }

        case InfoTimer:
        {
            Timer::Info *info = (Timer::Info *) addr;
            info->frequency = 250;
            info->ticks = 128;
            return API::Success;
        }

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

static API::Result hostVMCopyHandler(ProcessID procID, API::Operation how, Address ours,
                                     Address theirs, Size sz)
{
    if (procID == SELF || procID == (ProcessID)getpid())
    {
        switch (how)
        {
            case API::Read:
                MemoryBlock::copy((void *)ours, (void *)theirs, sz);
                return API::Success;

            case API::Write:
                MemoryBlock::copy((void *)theirs, (void *)ours, sz);
                return API::Success;

            default:
                break;
        }
    }

    return API::IOError;
}

static API::Result hostVMCtlHandler(ProcessID procID,
                                    MemoryOperation op,
                                    Memory::Range *range)
{
    switch (op)
    {
        case UnMap:
        {
            return API::Success;
        }

        case LookupVirtual:
        {
            range->phys = range->virt;
            return API::Success;
        }

        case MapContiguous:
        {
            if (range->virt == ZERO)
            {
                if (range->phys == ZERO)
                {
                    range->virt = (Address) new u8[range->size];
                }
                else
                {
                    range->virt = range->phys;
                }
            }
            return API::Success;
        }

        case Release:
        {
            if (range->virt != 0 && range->size != 0)
            {
                delete[] (u8 *)(range->virt);
            }
        }

        default:
            break;
    }

    return API::InvalidArgument;
}

static API::Result hostVMShareHandler(ProcessID procID,
                                      API::Operation op,
                                      ProcessShares::MemoryShare *share)
{
    switch (op)
    {
        case API::Create:
            return HostShareManager::instance()->createShare(procID, share);

        case API::Read:
            if (procID == SELF)
                return HostShareManager::instance()->readShare(share->pid, share);
            else
                return API::NotFound;

        case API::Delete:
            return HostShareManager::instance()->deleteShares(procID);

        default:
            break;
    }

    return API::InvalidArgument;
}

static API::Result hostApiHandler(ulong api, ulong arg1, ulong arg2, ulong arg3, ulong arg4, ulong arg5)
{
    switch (api)
    {
        case API::PrivExecNumber:
            return hostPrivExecHandler((PrivOperation) arg1, arg2);

        case API::ProcessCtlNumber:
            return hostProcessCtlHandler(arg1, (ProcessOperation) arg2, arg3, arg4);

        case API::SystemInfoNumber:
            return hostSystemInfoHandler((SystemInformation *) arg1);

        case API::VMCopyNumber:
            return hostVMCopyHandler(arg1, (API::Operation) arg2, arg3, arg4, arg5);

        case API::VMCtlNumber:
            return hostVMCtlHandler(arg1, (MemoryOperation) arg2, (Memory::Range *) arg3);

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
