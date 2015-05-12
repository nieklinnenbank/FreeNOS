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

#include <FreeNOS/API.h>
#include <FreeNOS/System/Constant.h>
#include <CoreServer.h>
#include <CoreMessage.h>
#include <ExecutableFormat.h>
#include <Types.h>
#include <Runtime.h>
#include <string.h>
#include <errno.h>
#include "unistd.h"

int forkexec(const char *path, const char *argv[])
{
    CoreMessage msg;
    ExecutableFormat *fmt;
    MemoryRegion regions[16];
    MemoryRange range;
    uint count = 0;
    pid_t pid = 0;
    int numRegions = 0;

    // Attempt to read executable format
    if (!(fmt = ExecutableFormat::find(path)))
        return -1;

    // Retrieve memory regions
    if ((numRegions = fmt->regions(regions, 16)) < 0)
        return -1;

    // We want to spawn a new process
    msg.action    = SpawnProcess;
    msg.path      = (char *) path;
    msg.number    = fmt->entry();
    
    // Ask CoreServer to create a new process
    IPCMessage(CORESRV_PID, API::SendReceive, &msg, sizeof(msg));

    // Obtain results
    errno = msg.result;
    pid   = msg.number;

    if (msg.result != ESUCCESS)
        return msg.result;
    
    /* Map program regions into virtual memory of the new process. */
    for (int i = 0; i < numRegions; i++)
    {
        /* Copy executable memory from this region. */
        for (Size j = 0; j < regions[i].size; j += PAGESIZE)
        {
            range.virtualAddress  = regions[i].virtualAddress + j;
            range.physicalAddress = ZERO;
            range.bytes = PAGESIZE;
        
            /* Create mapping first. */
            if (VMCtl(pid, Map, &range) != 0)
            {
                // TODO: convert from API::Error to errno.
                errno = EFAULT;
                return -1;
            }
            /* Copy bytes. */
            VMCopy(pid, API::Write, (Address) (regions[i].data) + j,
                   regions[i].virtualAddress + j, PAGESIZE);
        }
    }
    /* Create mapping for command-line arguments. */
    range.virtualAddress  = ARGV_ADDR;
    range.physicalAddress = ZERO;
    range.bytes = PAGESIZE;
    VMCtl(pid, Map, &range);

    // Allocate arguments
    char *arguments = new char[PAGESIZE];
    memset(arguments, 0, PAGESIZE);

    // Fill in arguments
    while (argv[count] && count < PAGESIZE / ARGV_SIZE)
    {
        strlcpy(arguments + (ARGV_SIZE * count), argv[count], ARGV_SIZE);
        count++;
    }

    // Copy argc/argv into the new process
    if ((VMCopy(pid, API::Write, (Address) arguments,
               (Address) ARGV_ADDR, PAGESIZE)) < 0)
    {
        delete arguments;
        errno = EFAULT;
        return -1;
    }

    // Begin execution
    ProcessCtl(pid, Resume);

    /* Cleanup. */
    delete arguments;
    
    /* All done. */
    return pid;
}
