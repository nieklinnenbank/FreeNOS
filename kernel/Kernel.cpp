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
#include <ListIterator.h>
#include <SplitAllocator.h>
#include <BubbleAllocator.h>
#include <PoolAllocator.h>
#include <BootImage.h>
#include "Kernel.h"
#include "Memory.h"
#include "Process.h"
#include "ProcessManager.h"
#include "Scheduler.h"
#include "API.h"

Kernel::Kernel(Memory::Range kernel, Memory::Range memory)
    : Singleton<Kernel>(this), m_interrupts(256)
{
    // Output log banners
    Log::instance->write(BANNER);
    Log::instance->write(COPYRIGHT "\r\n");

    // TODO: compute lower & higher memory for this core.
    Memory::Range highMem;
    MemoryBlock::set(&highMem, 0, sizeof(highMem));

    // Initialize members
    m_alloc  = new SplitAllocator(memory, highMem);
    m_procs  = new ProcessManager(new Scheduler());
    m_api    = new API();
    m_bootImageAddress = 0;

    // Mark kernel memory used (first 4MB in phys memory)
    for (Size i = 0; i < kernel.size; i += PAGESIZE)
        m_alloc->allocate(kernel.phys + i);

    // Clear interrupts table
    m_interrupts.fill(ZERO);
}

Error Kernel::heap(Address base, Size size)
{
    Allocator *bubble, *pool;
    Size meta = sizeof(BubbleAllocator) + sizeof(PoolAllocator);

    NOTICE("base = " << base << " size = " << size);

    // Clear the heap first
    MemoryBlock::set((void *) base, 0, size);

    // Setup the dynamic memory heap
    bubble = new (base) BubbleAllocator(base + meta, size - meta);
    pool   = new (base + sizeof(BubbleAllocator)) PoolAllocator();
    pool->setParent(bubble);

    // Set default allocator
    Allocator::setDefault(pool);
    return 0;
}

SplitAllocator * Kernel::getAllocator()
{
    return m_alloc;
}

ProcessManager * Kernel::getProcessManager()
{
    return m_procs;
}

API * Kernel::getAPI()
{
    return m_api;
}

Address Kernel::getBootImageAddress()
{
    return m_bootImageAddress;
}

Size Kernel::getBootImageSize()
{
    return m_bootImageSize;
}

MemoryContext * Kernel::getMemoryContext()
{
    return m_procs->current()->getMemoryContext();
}

void Kernel::hookInterrupt(u32 vec, InterruptHandler h, ulong p)
{
    InterruptHook hook(h, p);

    // Insert into interrupts; create List if neccesary
    if (!m_interrupts[vec])
    {
        m_interrupts.insert(vec, new List<InterruptHook *>());
    }
    // Just append it. */
    if (!m_interrupts[vec]->contains(&hook))
    {
        m_interrupts[vec]->append(new InterruptHook(h, p));
    }
}

void Kernel::executeInterrupt(u32 vec, CPUState *state)
{
    // Auto-Mask the IRQ. Any interrupt handler or user program
    // needs to re-enable the IRQ to receive it again. This prevents
    // interrupt loops in case the kernel cannot clear the IRQ immediately.
    enableIRQ(vec, false);

    // Fetch the list of interrupt hooks (for this vector)
    List<InterruptHook *> *lst = m_interrupts[vec];

    // Does at least one handler exist?
    if (!lst)
        return;

    // Execute them all
    for (ListIterator<InterruptHook *> i(lst); i.hasCurrent(); i++)
    {
        i.current()->handler(state, i.current()->param);
    }
}

bool Kernel::loadBootProcess(BootImage *image, Address imagePAddr, Size index)
{
    Address imageVAddr = (Address) image, args;
    Size args_size = ARGV_SIZE;
    BootSymbol *program;
    BootSegment *segment;
    Process *proc;
    char *vaddr;
    Arch::MemoryMap map;

    // Point to the program and segments table
    program = &((BootSymbol *) (imageVAddr + image->symbolTableOffset))[index];
    segment = &((BootSegment *) (imageVAddr + image->segmentsTableOffset))[program->segmentsOffset];

    // Ignore non-BootProgram entries
    if (program->type != BootProgram)
        return false;

    // Create process
    proc = m_procs->create(program->entry, map);
    if (!proc)
    {
        FATAL("failed to create boot program: " << program->name);
        return false;
    }

    proc->setState(Process::Ready);

    // Obtain process memory
    MemoryContext *mem = proc->getMemoryContext();

    // Map program segment into it's virtual memory
    for (Size i = 0; i < program->segmentsCount; i++)
    {
        for (Size j = 0; j < segment[i].size; j += PAGESIZE)
        {
            mem->map(segment[i].virtualAddress + j,
                     imagePAddr + segment[i].offset + j,
                     Memory::User     |
                     Memory::Readable |
                     Memory::Writable |
                     Memory::Executable);
        }
    }
    
    // Map program arguments into the process
    // TODO: move into the high memory???
    m_alloc->allocateLow(args_size, &args);
    mem->map(ARGV_ADDR, args, Memory::User | Memory::Readable | Memory::Writable);

    // Copy program arguments
    vaddr = (char *) m_alloc->toVirtual(args);
    MemoryBlock::set(vaddr, 0, PAGESIZE);
    MemoryBlock::copy(vaddr, program->name, ARGV_SIZE);

    // Done
    NOTICE("loaded: " << program->name);
    return true;
}

int Kernel::run()
{
    NOTICE("");

    // Load boot image programs
    loadBootImage();

    // Start the scheduler
    m_procs->schedule();

    // Never actually returns.
    return 0;
}
