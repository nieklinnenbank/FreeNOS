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

API::Result VMCtlHandler(const ProcessID procID,
                         const MemoryOperation op,
                         Memory::Range *range)
{
    ProcessManager *procs = Kernel::instance()->getProcessManager();
    MemoryContext::Result memResult = MemoryContext::Success;
    API::Result ret = API::Success;
    Process *proc = ZERO;

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

        case MapContiguous:
        case MapSparse:
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
            if (op == MapContiguous)
                memResult = mem->mapRangeContiguous(range);
            else
                memResult = mem->mapRangeSparse(range);

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

        case ReleaseSections:
            memResult = mem->releaseSection(*range);
            if (memResult != MemoryContext::Success)
            {
                ERROR("failed to release sections at virtual address " << (void *)range->virt <<
                      ": " << (int) memResult);
                return API::IOError;
            }
            break;

        case CacheClean: {
            Arch::Cache cache;
            cache.cleanData(range->virt);
            break;
        }

        case CacheInvalidate: {
            Arch::Cache cache;
            const Cache::Result r = cache.invalidateAddress(Cache::Data, range->virt);
            if (r != Cache::Success)
            {
                ERROR("failed to invalidate cache at address " << (void *) range->virt <<
                      ": result = " << (int) r);
                return API::IOError;
            }
            break;
        }

        case CacheCleanInvalidate: {
            Arch::Cache cache;
            cache.cleanInvalidate(Cache::Data);
            break;
        }

        case Access: {
            const MemoryContext::Result mr = mem->access(range->virt, &range->access);
            if (mr == MemoryContext::Success)
                ret = API::Success;
            else
                ret = API::AccessViolation;
            break;
        }

        case ReserveMem:
        {
            SplitAllocator *alloc = Kernel::instance()->getAllocator();
            Allocator::Result allocResult = Allocator::Success;

            for (Size i = 0; i < range->size; i += PAGESIZE)
            {
                const Address addr = range->phys + i;

                if (alloc->isAllocated(addr))
                {
                    ERROR("address " << (void *)addr << " is already allocated");
                    return API::InvalidArgument;
                }
                else if ((allocResult = alloc->allocate(addr)) != Allocator::Success)
                {
                    ERROR("failed to allocate " << (void *)addr << ", result = " << (int)allocResult);
                    return API::IOError;
                }
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
