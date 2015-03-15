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

#include <Log.h>
#include "Kernel.h"
#include "Memory.h"
#include "Process.h"
#include "ProcessManager.h"
#include <System/Multiboot.h>
#include <System/Constant.h>
#include "BootImage.h"
#include <UserProcess.h>
#include <String.h>

Kernel::Kernel(Memory *memory, ProcessManager *procs)
    : Singleton<Kernel>(this)
{
    DEBUG("");

    /* Initialize members */
    m_memory = memory;
    m_procs  = procs;

    /* Register generic API handlers */
    m_apis.fill(ZERO);
    m_apis.put(IPCMessageNumber, (APIHandler *) IPCMessageHandler);
    m_apis.put(PrivExecNumber,   (APIHandler *) PrivExecHandler);
    m_apis.put(ProcessCtlNumber, (APIHandler *) ProcessCtlHandler);
    m_apis.put(SystemInfoNumber, (APIHandler *) SystemInfoHandler);
    m_apis.put(VMCopyNumber,     (APIHandler *) VMCopyHandler);
    m_apis.put(VMCtlNumber,      (APIHandler *) VMCtlHandler);
    m_apis.put(IOCtlNumber,      (APIHandler *) IOCtlHandler);

    /* Load boot image programs */
    loadBootImage();
}

Memory * Kernel::getMemory()
{
    return m_memory;
}

ProcessManager * Kernel::getProcessManager()
{
    return m_procs;
}

void Kernel::run()
{
    DEBUG("");
    m_procs->schedule();
}

Error Kernel::invokeAPI(APINumber number,
                        ulong arg1, ulong arg2, ulong arg3, ulong arg4, ulong arg5)
{
    APIHandler **handler = (APIHandler **) m_apis.get(number);

    if (handler)
        return (*handler)(arg1, arg2, arg3, arg4, arg5);
    else
        return EINVAL;
}

bool Kernel::loadBootImage()
{
    MultibootModule *mod;
    BootImage *image;
    String str;
    bool found = false;

#warning Do not assume multiboot support for an architecture

    /* Startup boot modules. */
    for (Size n = 0; n < multibootInfo.modsCount; n++)
    {
        mod = &((MultibootModule *) multibootInfo.modsAddress)[n];

        /* Mark its memory used */
        for (Address a = mod->modStart; a < mod->modEnd; a += PAGESIZE)
        {
            m_memory->allocatePhysicalAddress(a);
        }

        /* Is this a BootImage? */
        if (str.match((char *) mod->string, "*.img.gz"))
        {
            /* Map the BootImage into our address space. */
            image = (BootImage *) m_memory->map(mod->modStart);
                                  m_memory->map(mod->modStart + PAGESIZE);

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
    proc = m_procs->create(program->entry);
    proc->setState(Process::Ready);
                    
    /* Loop program segments. */
    for (Size i = 0; i < program->segmentsCount; i++)
    {
        /* Map program segment into it's virtual memory. */
        for (Size j = 0; j < segment[i].size; j += PAGESIZE)
        {

            m_memory->map(proc,
                          imagePAddr + segment[i].offset + j,
                          segment[i].virtualAddress + j,
                          Memory::Present | Memory::User | Memory::Readable | Memory::Writable);
        }
    }
    /* Map and copy program arguments. */
    args = m_memory->allocatePhysical(PAGESIZE);
    m_memory->map(proc, args, ARGV_ADDR, Memory::Present | Memory::User | Memory::Writable);
    String::strlcpy( (char *) m_memory->map(args), program->path, ARGV_SIZE);
}
