/*
 * Copyright (C) 2025 Ivan Tan
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
#include "ARM64Process.h"

#define MEMALIGN16 16

static bool firstProcess = true;
extern u8 svcStack[PAGESIZE * 16];

ARM64Process::ARM64Process(ProcessID id, Address entry, bool privileged, const MemoryMap &map)
    : Process(id, entry, privileged, map)
{
}

Process::Result ARM64Process::initialize()
{
    Memory::Range range;
    Allocator::Range alloc_args;

    // Create MMU context
    m_memoryContext = new ARM64Paging(&m_map, Kernel::instance()->getAllocator());
    if (!m_memoryContext)
    {
        ERROR("failed to create memory context");
        return OutOfMemory;
    }

    // Initialize MMU context
    const MemoryContext::Result memResult = m_memoryContext->initialize();
    if (memResult != MemoryContext::Success)
    {
        ERROR("failed to initialize MemoryContext: result = " << (int) memResult);
        return OutOfMemory;
    }

    // Allocate User stack
    range = m_map.range(MemoryMap::UserStack);
    range.access = Memory::Readable | Memory::Writable | Memory::User;
    alloc_args.address = 0;
    alloc_args.size = range.size;
    alloc_args.alignment = PAGESIZE;

    if (Kernel::instance()->getAllocator()->allocate(alloc_args) != Allocator::Success)
    {
        ERROR("failed to allocate user stack");
        return OutOfMemory;
    }
    range.phys = alloc_args.address;

    // Map User stack
    if (m_memoryContext->mapRangeContiguous(&range) != MemoryContext::Success)
    {
        ERROR("failed to map user stack");
        return MemoryMapError;
    }

    // Fill usermode program registers
    reset(m_entry);

    // Finalize with generic initialization
    return Process::initialize();
}

ARM64Process::~ARM64Process()
{
}

const CPUState * ARM64Process::cpuState() const
{
    return &m_cpuState;
}

void ARM64Process::setCpuState(const CPUState *cpuState)
{
    MemoryBlock::copy(&m_cpuState, cpuState, sizeof(*cpuState));
}

ARM64Process::Result ARM64Process::join(const uint result)
{
    const Result r = Process::join(result);
    if (r == Success)
    {
        m_cpuState.x0 = API::Success | (result << 16);
    }

    return r;
}

void ARM64Process::reset(const Address entry)
{
    const Memory::Range range = m_map.range(MemoryMap::UserStack);

    MemoryBlock::set(&m_cpuState, 0, sizeof(m_cpuState));
    m_cpuState.sp = range.virt + range.size - MEMALIGN16;   // user stack pointer
    m_cpuState.pc = entry;                                  // user program counter
    m_cpuState.cpsr = (m_privileged ? 0x5 : 0x0);           // current program status (CPSR)
}

void ARM64Process::execute(Process *previous)
{
    DEBUG("proc " << previous->getID()  << " switch to proc " << getID() );
    // Activates memory context of this process
    m_memoryContext->activate();

    // First process starts from loadCoreState0
    if (firstProcess)
    {
        firstProcess = false;

        // Kernel stacks are currently 16KiB (see ARMBoot.S)
        CPUState *ptr = ((CPUState *) (svcStack + sizeof(svcStack))) - 1;
        MemoryBlock::copy(ptr, &m_cpuState, sizeof(*ptr));
        NOTICE("svcStack " << (void *)svcStack);

        // Switch to the actual SVC stack and switch to usermode
        asm volatile ("ldr x0, =(svcStack + (4096*16))\n"
                      "mov sp, x0\n"
                      "sub sp, sp, %0\n"
                      "ldr x0, =returnFromEL0Call8\n"
                      "br x0\n" : : "i" (sizeof(m_cpuState)) );
    }
}
