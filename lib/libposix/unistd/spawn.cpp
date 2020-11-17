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
#include <FileSystemClient.h>
#include <FileDescriptor.h>
#include <ExecutableFormat.h>
#include <Types.h>
#include <Runtime.h>
#include "limits.h"
#include "string.h"
#include "errno.h"
#include "unistd.h"

int spawn(Address program, Size programSize, const char *argv[])
{
    const FileSystemClient filesystem;
    ExecutableFormat *fmt;
    ExecutableFormat::Region regions[16];
    Arch::MemoryMap map;
    Memory::Range range;
    uint count = 0;
    pid_t pid = 0;
    Size numRegions = 16;
    Address entry;

    // Attempt to read executable format
    if (ExecutableFormat::find((u8 *) program, programSize, &fmt) != ExecutableFormat::Success)
    {
        errno = ENOEXEC;
        return -1;
    }

    // Find entry point
    if (fmt->entry(&entry) != ExecutableFormat::Success)
    {
        delete fmt;
        errno = ENOEXEC;
        return -1;
    }

    // Create new process
    const ulong result = ProcessCtl(ANY, Spawn, entry);
    if ((result & 0xffff) != API::Success)
    {
        delete fmt;
        errno = EIO;
        return -1;
    }
    pid = (result >> 16);

    // Retrieve memory regions
    if (fmt->regions(regions, &numRegions) != ExecutableFormat::Success)
    {
        delete fmt;
        errno = ENOEXEC;
        ProcessCtl(pid, KillPID);
        return -1;
    }
    // Release buffers
    delete fmt;

    // Map program regions into virtual memory of the new process
    for (Size i = 0; i < numRegions; i++)
    {
        // Setup memory range to copy region data
        range.virt   = regions[i].virt;
        range.phys   = ZERO;
        range.size   = regions[i].memorySize;
        range.access = regions[i].access;

        // Create mapping first in the new process
        if (VMCtl(pid, MapContiguous, &range) != API::Success)
        {
            errno = EFAULT;
            ProcessCtl(pid, KillPID);
            return -1;
        }

        // Map inside our process
        range.virt = ZERO;
        if (VMCtl(SELF, MapContiguous, &range) != API::Success)
        {
            errno = EFAULT;
            ProcessCtl(pid, KillPID);
            return -1;
        }

        // Copy data bytes
        MemoryBlock::copy((void *)range.virt, (const void *)(program + regions[i].dataOffset),
                          regions[i].dataSize);

        // Nulify remaining space
        if (regions[i].memorySize > regions[i].dataSize)
        {
            MemoryBlock::set((void *)(range.virt + regions[i].dataSize), 0,
                             regions[i].memorySize - regions[i].dataSize);
        }

        // Remove temporary mapping
        if (VMCtl(SELF, UnMap, &range) != API::Success)
        {
            errno = EFAULT;
            ProcessCtl(pid, KillPID);
            return -1;
        }
    }

    // Create mapping for command-line arguments
    range = map.range(MemoryMap::UserArgs);
    range.phys = ZERO;
    range.access = Memory::User | Memory::Readable | Memory::Writable;
    if (VMCtl(pid, MapContiguous, &range) != API::Success)
    {
        errno = EFAULT;
        ProcessCtl(pid, KillPID);
        return -1;
    }

    // Allocate arguments and current working directory
    char *arguments = new char[PAGESIZE*2];
    memset(arguments, 0, PAGESIZE*2);

    // Fill in arguments
    while (argv[count] && count < PAGESIZE / ARGV_SIZE)
    {
        strlcpy(arguments + (ARGV_SIZE * count), argv[count], ARGV_SIZE);
        count++;
    }

    // Fill in the current working directory
    strlcpy(arguments + PAGESIZE, **filesystem.getCurrentDirectory(), PATH_MAX);

    // Copy argc/argv into the new process
    if (VMCopy(pid, API::Write, (Address) arguments, range.virt, PAGESIZE * 2) != API::Success)
    {
        delete[] arguments;
        errno = EFAULT;
        ProcessCtl(pid, KillPID);
        return -1;
    }

    // Copy fds into the new process.
    if (VMCopy(pid, API::Write, (Address) FileDescriptor::instance()->getArray(count),
               range.virt + (PAGESIZE * 2), range.size - (PAGESIZE * 2)) != API::Success)
    {
        delete[] arguments;
        errno = EFAULT;
        ProcessCtl(pid, KillPID);
        return -1;
    }

    // Let the Child begin execution
    if (ProcessCtl(pid, Resume) != API::Success)
    {
        delete[] arguments;
        errno = EIO;
        ProcessCtl(pid, KillPID);
        return -1;
    }

    // Done. Cleanup.
    delete[] arguments;
    return pid;
}
