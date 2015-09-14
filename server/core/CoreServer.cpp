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

#include <FreeNOS/API.h>
#include <FreeNOS/System.h>
#include <ExecutableFormat.h>
#include "CoreServer.h"
#include "CoreMessage.h"
#include <stdio.h>
#include <string.h>

#ifdef INTEL
#include <intel/IntelMP.h>
#endif

const char * CoreServer::kernelPath = "/boot/kernel";

CoreServer::CoreServer()
    : IPCServer<CoreServer, CoreMessage>(this)
{
    /*
     * TODO:
     * discover other CPUs with libarch APIC. Determine the memory
     * splitup. Claim the memory for that CPU. Fill the boot struct with various argument
     * inside the cpu1 memory so that IntelBoot.S can find its base.
     * start new kernel with /boot/kernel (or any other kernel, depending on configuration)
     * introduce a IntelGeometry, which uses APIC. CoreServer uses the Arch::Geometry to discover CPUs here.
     * once CPU1 is up & running, we can implement libmpi! :-)
     */
    m_numRegions = 0;
    m_kernel = ZERO;
}

CoreServer::Result CoreServer::initialize()
{
    SystemInformation info;

    // Only core0 needs to start other coreservers
    if (info.coreId != 0)
        return Success;

    Result r = loadKernel();

    if (r == Success)
        return discover();
    else
        return r;
}

CoreServer::Result CoreServer::loadKernel()
{
    DEBUG("Opening : " << kernelPath);

    // Attempt to read kernel executable format
    if (!(m_kernel = ExecutableFormat::find(kernelPath)))
    {
        ERROR("kernel not found: " << kernelPath << ": " << strerror(errno));
        return NotFound;
    }
    DEBUG("Reading : " << kernelPath);

    // Retrieve memory regions
    if ((m_numRegions = m_kernel->regions(m_regions, 16)) < 0)
    {
        ERROR("kernel not usable: " << kernelPath << ": " << strerror(errno));
        return ExecError;
    }
    DEBUG("kernel loaded");
    return Success;
}

CoreServer::Result CoreServer::bootCore(uint coreId, CoreInfo *info,
                                        MemoryRegion *regions)
{
    SystemInformation sysInfo;
    DEBUG("Reserving: " << (void *)info->memory.phys << " size=" <<
            info->memory.size << " available=" << sysInfo.memoryAvail);

    // Claim the core's memory
    if (VMCtl(SELF, RemoveMem, &info->memory) != API::Success)
    {
        ERROR("failed to reserve memory for core" << coreId <<
              " at " << (void *)info->memory.phys);
        return OutOfMemory;
    }

    DEBUG("Starting core" << coreId << " with "
          << info->memory.size / 1024 / 1024 << "MB");

    for (int i = 0; i < m_numRegions; i++)
    {
        Memory::Range range;
        range.phys = info->memory.phys + regions[i].virtualAddress;
        range.virt = 0;
        range.size = regions[i].size;
        range.access = Memory::Readable | Memory::Writable |
                       Memory::User;

        // Map the target kernel's memory for regions[i].size
        if (VMCtl(SELF, Map, &range) != 0)
        {
            // TODO: convert from API::Error to errno.
            //errno = EFAULT;
            return OutOfMemory;
        }
        // Copy the kernel to the target core's memory
#warning VMCopy should just return API::Result, not a Size
	Error r = VMCopy(SELF, API::Write, (Address) regions[i].data,
                                 range.virt,
                                 regions[i].size);
	if (r != regions[i].size)
	    return MemoryError;

        // Unmap the target kernel's memory
        if (VMCtl(SELF, UnMap, &range) != API::Success)
        {
            return MemoryError;
        }

        DEBUG(kernelPath << "[" << i << "] = " << (void *) m_regions[i].virtualAddress);
    }

    // Copy the BootImage after the kernel.
    Memory::Range range;
    range.phys = info->bootImageAddress;
    range.virt = 0;
    range.size = info->bootImageSize;
    range.access = Memory::Readable | Memory::Writable | Memory::User;

    // Map BootImage buffer
    if (VMCtl(SELF, Map, &range) != API::Success)
    {
        return OutOfMemory;
    }
    // Copy the BootImage
    Error r = VMCopy(SELF, API::Write, sysInfo.bootImageAddress,
                     range.virt, sysInfo.bootImageSize);
    if (r != (Error) sysInfo.bootImageSize)
        return MemoryError;

    // Unmap the BootImage
    if (VMCtl(SELF, UnMap, &range) != API::Success)
        return MemoryError;

#ifdef INTEL
    // Signal the core to boot
    if (m_cores.boot(info) != IntelMP::Success) {
        ERROR("failed to boot core" << coreId);
        return BootError;
    } else {
        NOTICE("core" << coreId << " started");
    }
#endif
    return Success;
}

CoreServer::Result CoreServer::discover()
{
#ifdef INTEL
    CoreInfo info;
    SystemInformation sysInfo;
    Size memPerCore = 0;

    m_cores.discover();
    List<uint> & cores = m_cores.getCores();
    memPerCore = sysInfo.memorySize / cores.count();
    memPerCore /= MegaByte(4);
    memPerCore *= MegaByte(4);

    NOTICE("found " << cores.count() << " cores -- " <<
            (memPerCore / 1024 / 1024) << "MB per core");

    for (ListIterator<uint> i(cores); i.hasCurrent(); i++)
    {
        uint coreId = i.current();

        if (coreId != 0)
        {
            MemoryBlock::set(&info, 0, sizeof(info));
            info.coreId = coreId;
            info.memory.phys = memPerCore * coreId;
            info.memory.size = memPerCore - PAGESIZE;
            info.kernel.phys = info.memory.phys;
            info.kernel.size = MegaByte(4);
            info.bootImageAddress = info.kernel.phys + info.kernel.size;
            info.bootImageSize    = sysInfo.bootImageSize;
            info.kernelEntry  = m_kernel->entry();
            info.timerCounter = sysInfo.timerCounter;
            strlcpy(info.kernelCommand, kernelPath, KERNEL_PATHLEN);

            bootCore(coreId, &info, m_regions);
        }
    }
#endif
    return Success;
}
