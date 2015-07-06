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
#include <BitAllocator.h>
#include "ARMProcess.h"

ARMProcess::ARMProcess(ProcessID id, Address entry, bool privileged)
    : Process(id, entry, privileged)
{
    NOTICE("id =" << id << " entry =" << entry << " privileged = " << privileged);

    Size size = PAGEDIR_SIZE;
    BitAllocator *memory = Kernel::instance->getMemory();
    Memory::Range range;
    Size framesize = (14+17)*sizeof(u32);

    // Allocate first level page table
    memory->allocate(&size, &m_pageDirectory, KiloByte(16));

    // Initialize memory context
    Arch::Memory mem(m_pageDirectory, memory);
    mem.create();

    // User stack.
    range.phys   = 0;
    range.virt   = mem.range(Memory::UserStack).virt;
    range.size   = mem.range(Memory::UserStack).size;
    range.access = Memory::Present |
                   Memory::User |
                   Memory::Readable |
                   Memory::Writable;
    mem.mapRange(&range);
    setUserStack(range.virt + range.size - MEMALIGN);

    // Kernel stack.
    range.phys   = 0;
    range.virt   = mem.range(Memory::KernelStack).virt;
    range.size   = mem.range(Memory::KernelStack).size;
    range.access = Memory::Present | Memory::Readable | Memory::Writable;
    mem.mapRange(&range);
    setKernelStack(range.virt + range.size - MEMALIGN - framesize);

    // Map kernel stack.
    Arch::Memory local(0, memory);
    range.virt = local.findFree(range.size, Memory::KernelPrivate);
    local.mapRange(&range);
    Address *stack = (Address *) (range.virt + range.size - framesize - MEMALIGN);

    // Zero kernel stack
    MemoryBlock::set((void *)range.virt, 0, range.size);

    // restoreState: fill kernel register state
    stack[0] = (Address) loadCoreState0; /* restoreState: pop {lr} */
    stack += 14;

    // loadCoreState0: fill user register state
    stack[0] = (privileged ? SYS_MODE : USR_MODE) | FIQ_BIT | IRQ_BIT; /* user program status (CPSR) */
    stack++;
    stack[0] = m_userStack; /* user program SP */
    stack[1] = 0;           /* user program LR */
    stack+=15;
    stack[0] = entry;       /* user program entry (PC) */

    local.unmapRange(&range);
}

ARMProcess::~ARMProcess()
{
    Arch::Memory mem(getPageDirectory(), Kernel::instance->getMemory());

    // Release regions
    mem.releaseRegion(Memory::KernelStack);
    mem.releaseRegion(Memory::UserData);
    mem.releaseRegion(Memory::UserHeap);
    mem.releaseRegion(Memory::UserStack);
    mem.releaseRegion(Memory::UserPrivate);
}

void ARMProcess::execute(Process *previous)
{
    ARMProcess *p = (ARMProcess *) previous;

    switchCoreState( p ? &p->m_kernelStack : ZERO,
                     m_pageDirectory,
                     m_kernelStack );
}
