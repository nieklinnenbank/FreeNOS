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

#include <arch/Kernel.h>
#include <arch/Process.h>
#include <arch/Scheduler.h>
#include <arch/Multiboot.h>

Kernel::Kernel()
{
    MultibootModule *mod;
    ArchProcess *modProc;
    Address vstart = 0x80000000;
    Size modSize;

    /* Startup modules. */
    for (Size n = 0; n <  multibootInfo.modsCount; n++)
    {
	mod     = &((MultibootModule *) multibootInfo.modsAddress)[n];
	modProc = new ArchProcess(vstart);
	modSize = mod->modEnd - mod->modStart;
	
	/* Map the module in virtual memory. */
	for (Size i = 0; i < modSize; i += PAGESIZE)
	{
	    memory->mapVirtual(modProc, mod->modStart + i, vstart + i,
			       PAGE_PRESENT|PAGE_USER|PAGE_RW);
	}
	/* HACK: allow VGA access. */
	memory->mapVirtual(modProc, 0xb8000, 0x70000000,
			   PAGE_PRESENT|PAGE_USER|PAGE_RW|PAGE_PINNED);
	
	/* Schedule the process. */
	modProc->setState(Ready);
	scheduler->enqueue(modProc);
    }
}
