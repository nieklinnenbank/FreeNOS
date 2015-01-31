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

#include "Kernel.h"
#include <Arch/Memory.h>
#include "Process.h"
#include "Scheduler.h"
#include "Multiboot.h"
#include "BootImage.h"
#include <UserProcess.h>
#include <String.h>

Kernel::Kernel()
{
}

bool Kernel::loadBootImage()
{
    MultibootModule *mod;
    BootImage *image;
    String str;
    bool found = false;

    /* Startup boot modules. */
    for (Size n = 0; n <  multibootInfo.modsCount; n++)
    {
        mod = &((MultibootModule *) multibootInfo.modsAddress)[n];

        /* Is this a BootImage? */
        if (str.match((char *) mod->string, "*.img.gz"))
        {
            /* Map the BootImage into our address space. */
            image = (BootImage *) memory->mapVirtual(mod->modStart);
                                  memory->mapVirtual(mod->modStart + PAGESIZE);
            found = true;        

            /* Verify this is a correct BootImage. */
            if (image->magic[0] == BOOTIMAGE_MAGIC0 &&
                image->magic[1] == BOOTIMAGE_MAGIC1 &&
                image->layoutRevision == BOOTIMAGE_REVISION)
            {       
                /* Loop BootPrograms. */
                for (Size i = 0; i < image->programsTableCount; i++)
                {
                    loadBootProcess(image, mod->modStart, i);
                }
            }
        }
    }
    /* Done */
    return found;
}

void Kernel::loadBootProcess(BootImage *image, Address imagePAddr, Size index)
{
    Address imageVAddr = (Address) image, args;
    BootProgram *program;
    BootSegment *segment;
    Process *proc;
    
    /* Point to the program and segments table. */
    program = &((BootProgram *) (imageVAddr + image->programsTableOffset))[index];
    segment = &((BootSegment *) (imageVAddr + image->segmentsTableOffset))[program->segmentsOffset];

    /* Create process. */
    proc = createProcess(program->entry);
    proc->setState(Ready);
                    
    /* Loop program segments. */
    for (Size i = 0; i < program->segmentsCount; i++)
    {
        /* Map program segment into it's virtual memory. */
        for (Size j = 0; j < segment[i].size; j += PAGESIZE)
        {
            memory->mapVirtual(proc,
                               imagePAddr + segment[i].offset + j,
                               segment[i].virtualAddress + j,
                               PAGE_PRESENT | PAGE_USER | PAGE_RW);
        }
    }
    /* Map and copy program arguments. */
    args = memory->allocatePhysical(PAGESIZE);
    memory->mapVirtual(proc, args, ARGV_ADDR, PAGE_PRESENT | PAGE_USER | PAGE_RW);
    String::strlcpy( (char *) memory->mapVirtual(args), program->path, ARGV_SIZE);
    
    /* Schedule process. */
    scheduler->enqueue(proc);
}
