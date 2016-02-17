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
#include <Log.h>
#include <SplitAllocator.h>
#include "ARMProcess.h"
#define MEMALIGN8 8

ARMProcess::ARMProcess(ProcessID id, Address entry, bool privileged, const MemoryMap &map)
    : Process(id, entry, privileged, map)
{
    // TODO: set some members
}

Process::Result ARMProcess::initialize()
{
    Memory::Range range;
    Size framesize = (14+17)*sizeof(u32);
    Address stackAddr;

    // Create MMU context
    m_memoryContext = new ARMPaging(
        &m_map,
        Kernel::instance->getAllocator()
    );
    if (!m_memoryContext)
        return OutOfMemory;

    // User stack (high memory).
    range = m_map.range(MemoryMap::UserStack);
    range.access = Memory::Readable | Memory::Writable | Memory::User;
    if (Kernel::instance->getAllocator()->allocate(&range.size, &range.phys) != Allocator::Success)
        return OutOfMemory;

    if (m_memoryContext->mapRange(&range) != MemoryContext::Success)
        return MemoryMapError;
    setUserStack(range.virt + range.size - MEMALIGN8);

    // Kernel stack (low memory).
    Size stackSize = PAGESIZE;
    if (Kernel::instance->getAllocator()->allocateLow(stackSize, &stackAddr) != Allocator::Success)
        return OutOfMemory;

    stackAddr = (Address) Kernel::instance->getAllocator()->toVirtual(stackAddr);
    m_kernelStackBase = stackAddr + stackSize;
    setKernelStack(m_kernelStackBase - framesize - MEMALIGN8);
    Address *stack = (Address *) m_kernelStack;

    // Zero kernel stack
    MemoryBlock::set((void *) stackAddr, 0, stackSize);

    // restoreState: fill kernel register state
    stack[0] = (Address) loadCoreState0; /* restoreState: pop {lr} */
    stack += 14;

    // loadCoreState0: fill user register state
    stack[0] = (m_privileged ? SYS_MODE : USR_MODE); /* user program status (CPSR) */
    stack++;
    stack[0] = m_userStack; /* user program SP */
    stack[1] = 0;           /* user program LR */
    stack+=15;
    stack[0] = m_entry;     /* user program entry (PC) */

    // Finalize with generic initialization
    return Process::initialize();
}

ARMProcess::~ARMProcess()
{
}

void ARMProcess::execute(Process *previous)
{
    ARMProcess *p = (ARMProcess *) previous;

    // No need to assign the process kernel stack somewhere (compared to TSS.esp0 for Intel).
    // ARM cores have banked registers for each ARM mode. Once the stack
    // register is set in the current ARM mode (supervisor), it will be saved
    // in the banked (copied) stack register. When user code interrupts, the
    // banked register will be applied for the stack automatically.

    // Activate the memory context of this process
    m_memoryContext->activate();

    switchCoreState( p ? &p->m_kernelStack : ZERO,
                     m_kernelStack );
}
