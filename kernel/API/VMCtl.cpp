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
#include <FreeNOS/ProcessManager.h>
#include <SplitAllocator.h>
#include "VMCtl.h"
#include "ProcessID.h"

API::Result VMCtlHandler(ProcessID procID, MemoryOperation op, Memory::Range *range)
{
    ProcessManager *procs = Kernel::instance->getProcessManager();
    Process *proc = ZERO;
    MemoryContext::Result memResult = MemoryContext::Success;
    Error ret = API::Success;

    DEBUG("");

    // Find the given process
    if (procID == SELF)
        proc = procs->current();
    else if (!(proc = procs->get(procID)))
    {
        return API::NotFound;
    }

    // Retrieve memory context
    MemoryContext *mem = proc->getMemoryContext();

    // Perform operation
    switch (op)
    {
        case LookupVirtual:
            // Translate virtual address to physical address (page boundary)
            memResult = mem->lookup(range->virt, &range->phys);
            if (memResult != MemoryContext::Success)
            {
                ERROR("failed to lookup virtual address " << (void *) range->virt <<
                      ": " << (int) memResult);
                return API::AccessViolation;
            }
            assert(!(range->phys & ~PAGEMASK));

            // Add offset within the page
            range->phys += range->virt & ~PAGEMASK;
            break;

        case Map:
            if (!range->virt)
            {
                memResult = mem->findFree(range->size, MemoryMap::UserPrivate, &range->virt);
                if (memResult != MemoryContext::Success)
                {
                    ERROR("failed to find free virtual address in UserPrivate: " <<
                         (int) memResult);
                    return API::IOError;
                }
                range->virt += range->phys & ~PAGEMASK;
            }
            memResult = mem->mapRange(range);
            if (memResult != MemoryContext::Success)
            {
                ERROR("failed to map memory range " << (void *)range->virt << "->" <<
                     (void *) range->phys << ": " << (int) memResult);
                return API::IOError;
            }
            break;

        case UnMap:
            memResult = mem->unmapRange(range);
            if (memResult != MemoryContext::Success)
            {
                ERROR("failed to unmap range at virtual address " << (void *)range->virt <<
                      ": " << (int) memResult);
                return API::IOError;
            }
            break;

        case Release:
            memResult = mem->releaseRange(range);
            if (memResult != MemoryContext::Success)
            {
                ERROR("failed to release range at virtual address " << (void *)range->virt <<
                      ": " << (int) memResult);
                return API::IOError;
            }
            break;

        case CacheClean: {
            Arch::Cache cache;
            cache.cleanInvalidate(Cache::Data);
            break;
        }

        case Access: {
            MemoryContext::Result mr = mem->access(range->virt, &range->access);
            if (mr == MemoryContext::Success)
                ret = API::Success;
            else
                ret = (API::Result) mr;
            break;
        }

        case ReserveMem: {
            Allocator::Range alloc_args;
            alloc_args.address = range->phys;
            alloc_args.size    = range->size;
            alloc_args.alignment = PAGESIZE;

            if (Kernel::instance->getAllocator()->allocate(alloc_args) != Allocator::Success)
            {
                ERROR("address " << (void *) (range->phys) << " already allocated");
                return API::OutOfMemory;
            }
            break;
        }

        default:
            ret = API::InvalidArgument;
            break;
    }
    // Done
    return ret;
}
