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
#include <Log.h>
#include "IntelConstant.h"
#include "IntelMP.h"
#include "IntelBoot.h"

IntelMP::IntelMP()
{
    m_io.map(MPAreaAddr, MPAreaSize);
}

List<uint> & IntelMP::getCores()
{
    return m_cores;
}

IntelMP::Result IntelMP::discover()
{
    MPConfig *mpc = 0;
    MPEntry *entry;
    MPFloat *mpf;
    Address addr = m_io.getBase();

    // Clear previous discoveries
    m_cores.clear();

    // Look for the Multiprocessor configuration
    for (uint i = 0; i < MPAreaSize - sizeof(Address); i += sizeof(Address))
    {
        mpf = (MPFloat *)(addr + i);

        if (mpf->signature == MPFloatSignature)
        {
	    NOTICE("mpf = " << (void *) mpf);
            mpc = (MPConfig *) (mpf->configAddr - MPAreaAddr + addr);
            break;
        }
    }
    // MPTables not found
    if (!mpc)
        return NotFound;

    // Found config
    NOTICE("MP header found at " << mpc);
    NOTICE("Local APIC at " << (void *) mpc->apicAddr);
    entry = (MPEntry *)(mpc + 1);

    // Search for multiprocessor entries
    for (uint i = 0; i < mpc->count; i++)
        entry = parseEntry(entry);

    // Remap APIC in virtual memory
    m_apic.getIO().map(mpc->apicAddr, PAGESIZE);

    return Success;
}

IntelMP::Result IntelMP::boot(CoreInfo *info)
{
    NOTICE("booting core#" << info->coreId << " at " <<
            (void *) info->memory.phys << " with kernel: " << info->kernel);

    // TODO: load the kernel, reserve memory, etc
    // TODO: upper layer should have loaded the kernel in memory already.

    // Copy 16-bit realmode startup code
    // TODO: place this in the kernel binary somewhere instead?
    VMCopy(SELF, API::Write, (Address) bootEntry16, MPEntryAddr, PAGESIZE);

    // Copy the CoreInfo structure
    VMCopy(SELF, API::Write, (Address) info, MPInfoAddr, sizeof(*info));

    // Send inter-processor-interrupt to wakeup the processor
    if (m_apic.sendStartupIPI(info->coreId, MPEntryAddr) != IntelAPIC::Success)
        return IOError;

    // Wait until the core raises the 'booted' flag in CoreInfo
    // TODO: set somekind of limit to wait???
    while (1)
    {
        CoreInfo check;

        VMCopy(SELF, API::Read, (Address) &check, MPInfoAddr, sizeof(check));

        if (check.booted)
            break;
    }
    return Success;
}

IntelMP::MPEntry * IntelMP::parseEntry(IntelMP::MPEntry *entry)
{
    if (entry->type == MPEntryProc)
    {
        m_cores.append(entry->apicId);
        return entry + 1;
    }
    else
        return (MPEntry *) (((Address)(entry)) + 8);
}
