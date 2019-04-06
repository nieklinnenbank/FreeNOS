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
}

Process::Result ARMProcess::initialize()
{
    Memory::Range range;

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

    // Fill usermode program registers
    MemoryBlock::set(&m_cpuState, 0, sizeof(m_cpuState));
    m_cpuState.sp = m_userStack;  /* user stack pointer */
    m_cpuState.pc = m_entry;      /* user program counter */
    m_cpuState.cpsr = (m_privileged ? SYS_MODE : USR_MODE); /* current program status (CPSR) */

    // Finalize with generic initialization
    return Process::initialize();
}

ARMProcess::~ARMProcess()
{
}

const CPUState * ARMProcess::cpuState() const
{
    return &m_cpuState;
}

void ARMProcess::setCpuState(const CPUState *cpuState)
{
    MemoryBlock::copy(&m_cpuState, cpuState, sizeof(*cpuState));
}

static bool firstProcess = true;
extern u8 svcStack[PAGESIZE];

void ARMProcess::execute(Process *previous)
{
    // Activate the memory context of this process
    m_memoryContext->activate();

    // First process starts from loadCoreState0
    if (firstProcess)
    {
        firstProcess = false;

        CPUState *ptr = ((CPUState *) (svcStack)) - 1;
        MemoryBlock::copy(ptr, &m_cpuState, sizeof(*ptr));

        /* Switch to the actual SVC stack and switch to usermode */
        asm volatile ("ldr sp, =svcStack\n"
                      "sub sp, sp, %0\n"
                      "ldr r0, =loadCoreState0\n"
                      "bx r0\n" : : "i" (sizeof(m_cpuState) - sizeof(m_cpuState.padding)) );
    }
}
