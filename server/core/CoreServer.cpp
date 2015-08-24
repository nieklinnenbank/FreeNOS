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
#include <intel/IntelMP.h>
#include "CoreServer.h"
#include "CoreMessage.h"
#include <stdio.h>
#include <string.h>

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

#ifdef INTEL
    IntelMP mp;
    mp.discover();

    List<uint> & cpus = mp.getCPUs();
    NOTICE("found " << cpus.count() << " cores");
#endif

}
