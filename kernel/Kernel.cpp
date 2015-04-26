/*
 * Copyright (C) 2009 Niek Linnenbank
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

#include <Log.h>
#include "Kernel.h"
#include "Memory.h"
#include "Process.h"
#include "ProcessManager.h"
#include <System/Constant.h>

Kernel::Kernel(Memory *memory, ProcessManager *procs)
    : Singleton<Kernel>(this)
{
    NOTICE("");

    // Initialize members
    m_memory = memory;
    m_procs  = procs;
    m_bootImageAddress = 0;

    // Register generic API handlers
    m_apis.fill(ZERO);
    m_apis.insert(IPCMessageNumber, (APIHandler *) IPCMessageHandler);
    m_apis.insert(PrivExecNumber,   (APIHandler *) PrivExecHandler);
    m_apis.insert(ProcessCtlNumber, (APIHandler *) ProcessCtlHandler);
    m_apis.insert(SystemInfoNumber, (APIHandler *) SystemInfoHandler);
    m_apis.insert(VMCopyNumber,     (APIHandler *) VMCopyHandler);
    m_apis.insert(VMCtlNumber,      (APIHandler *) VMCtlHandler);
    m_apis.insert(IOCtlNumber,      (APIHandler *) IOCtlHandler);
}

Memory * Kernel::getMemory()
{
    return m_memory;
}

ProcessManager * Kernel::getProcessManager()
{
    return m_procs;
}

Address Kernel::getBootImageAddress()
{
    return m_bootImageAddress;
}

Size Kernel::getBootImageSize()
{
    return m_bootImageSize;
}

void Kernel::run()
{
    NOTICE("");

    // Load boot image programs
    loadBootImage();

    // Start the scheduler
    m_procs->schedule();
}

Error Kernel::invokeAPI(APINumber number,
                        ulong arg1, ulong arg2, ulong arg3, ulong arg4, ulong arg5)
{
    APIHandler **handler = (APIHandler **) m_apis.get(number);

    if (handler)
        return (*handler)(arg1, arg2, arg3, arg4, arg5);
    else
        return EINVAL;
}
