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

#include <Log.h>
#include "IntelMP.h"

IntelMP::IntelMP()
{
    m_io.map(MPAreaAddr, MPAreaSize);
}

List<uint> & IntelMP::getCPUs()
{
    return m_cpus;
}

IntelMP::Result IntelMP::discover()
{
    MPConfig *mpc = 0;
    MPEntry *entry;
    MPFloat *mpf;
    Address addr = m_io.getBase();

    // Clear previous discoveries
    m_cpus.clear();

    // Look for the Multiprocessor configuration
    for (uint i = 0; i < MPAreaSize; i += sizeof(Address))
    {
        mpf = (MPFloat *)(addr + i);

        if (mpf->signature == MPFloatSignature)
        {
            mpc = (MPConfig *) mpf->configAddr;
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

    return Success;
}

IntelMP::MPEntry * IntelMP::parseEntry(IntelMP::MPEntry *entry)
{
    if (entry->type == MPEntryProc)
    {
        m_cpus.append(entry->apicId);
        return entry + 1;
    }
    else
        return (MPEntry *) (((Address)(entry)) + 8);
}
