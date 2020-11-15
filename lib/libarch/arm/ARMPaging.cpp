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
#include <SplitAllocator.h>
#include <MemoryBlock.h>
#include <Log.h>
#include "CoreInfo.h"
#include "ARMCore.h"
#include "ARMControl.h"
#include "ARMPaging.h"
#include "ARMFirstTable.h"

ARMPaging::ARMPaging(MemoryMap *map, SplitAllocator *alloc)
    : MemoryContext(map, alloc)
    , m_firstTable(0)
    , m_firstTableAddr(0)
    , m_kernelBaseAddr(coreInfo.memory.phys)
{
}

ARMPaging::~ARMPaging()
{
    if (m_firstTableAddr != 0)
    {
        for (Size i = 0; i < sizeof(ARMFirstTable); i += PAGESIZE)
            m_alloc->release(m_firstTableAddr + i);
    }
}

ARMPaging::ARMPaging(MemoryMap *map,
                     Address firstTableAddress,
                     Address kernelBaseAddress)
    : MemoryContext(map, ZERO)
    , m_firstTable((ARMFirstTable *) firstTableAddress)
    , m_firstTableAddr(firstTableAddress)
    , m_kernelBaseAddr(kernelBaseAddress)
{
}

MemoryContext::Result ARMPaging::initialize()
{
    // Allocate first page table if needed
    if (m_firstTable == 0)
    {
        Allocator::Range phys, virt;
        phys.address = 0;
        phys.size = sizeof(ARMFirstTable);
        phys.alignment = sizeof(ARMFirstTable);

        // Allocate page directory
        if (m_alloc->allocate(phys, virt) != Allocator::Success)
        {
            return MemoryContext::OutOfMemory;
        }

        m_firstTable = (ARMFirstTable *) virt.address;
        m_firstTableAddr = phys.address;
    }

    // Initialize the page directory
    MemoryBlock::set(m_firstTable, 0, sizeof(ARMFirstTable));

    // Map the kernel. The kernel has permanently mapped 1GB of
    // physical memory. This 1GiB memory region starts at its physical
    // base address offset which varies per core.
    Memory::Range kernelRange = m_map->range(MemoryMap::KernelData);
    kernelRange.phys = m_kernelBaseAddr;
    m_firstTable->mapLarge(kernelRange, m_alloc);

#ifndef BCM2835
    // Temporary stack is used for kernel initialization code
    // and for SMP the temporary stack is shared between cores.
    // This is needed in order to perform early-MMU enable.
    m_firstTable->unmap(TMPSTACKADDR, m_alloc);

    const Memory::Range tmpStackRange = {
        TMPSTACKADDR, TMPSTACKADDR, MegaByte(1), Memory::Readable|Memory::Writable
    };
    m_firstTable->mapLarge(tmpStackRange, m_alloc);
#endif /* BCM2835 */

    // Unmap I/O zone
    for (Size i = 0; i < IO_SIZE; i += MegaByte(1))
        m_firstTable->unmap(IO_BASE + i, m_alloc);

    // Map the I/O zone as Device / Uncached memory.
    Memory::Range io;
    io.phys = IO_BASE;
    io.virt = IO_BASE;
    io.size = IO_SIZE;
    io.access = Memory::Readable | Memory::Writable | Memory::Device;
    m_firstTable->mapLarge(io, m_alloc);

    return MemoryContext::Success;
}

#ifdef ARMV6
MemoryContext::Result ARMPaging::enableMMU()
{
    ARMControl ctrl;

    // Program first level table. Enable L2 cache for page walking.
    ctrl.write(ARMControl::TranslationTable0, ((u32) m_firstTableAddr | 1));
    ctrl.write(ARMControl::TranslationTable1,    0);
    ctrl.write(ARMControl::TranslationTableCtrl, 0);

    // Set Control flags
    ctrl.set(ARMControl::DomainClient);
    ctrl.set(ARMControl::DisablePageColoring);
    ctrl.set(ARMControl::AccessPermissions);
    ctrl.set(ARMControl::ExtendedPaging);
    ctrl.unset(ARMControl::BranchPrediction);

    // Flush TLB's and caches
    tlb_flush_all();
    m_cache.cleanInvalidate(Cache::Unified);

    // Disable caches.
    ctrl.unset(ARMControl::InstructionCache);
    ctrl.unset(ARMControl::DataCache);

    // Enable the MMU. This re-enables instruction and data cache too.
    ctrl.set(ARMControl::MMUEnabled);
    tlb_flush_all();

    // Reactivate both caches and branch prediction
    ctrl.set(ARMControl::InstructionCache);
    ctrl.set(ARMControl::DataCache);
    ctrl.set(ARMControl::BranchPrediction);

    return Success;
}

#elif defined(ARMV7)

MemoryContext::Result ARMPaging::enableMMU()
{
    ARMControl ctrl;

    // Flush TLB's
    tlb_flush_all();
    dsb();
    isb();

    // Enable branch prediction
    ctrl.set(ARMControl::BranchPrediction);

    // Program first level table
    ctrl.write(ARMControl::TranslationTable0, (((u32) m_firstTableAddr) |
        (1 << 3) | // outer write-back, write-allocate
        (1 << 6)   // inner write-back, write-allocate
    ));
    ctrl.write(ARMControl::TranslationTable1,    0);
    ctrl.write(ARMControl::TranslationTableCtrl, 0);
    dsb();
    isb();

    // Set as client for all domains
    ctrl.write(ARMControl::DomainControl, 0x55555555);

    // Enable the MMU.
    u32 nControl = ctrl.read(ARMControl::SystemControl);

    // Raise all caching, MMU and branch prediction flags.
    nControl |= (1 << 11) | (1 << 2) | (1 << 12) | (1 << 0) | (1 << 5);

    // Write back to set.
    ctrl.write(ARMControl::SystemControl, nControl);
    isb();

    // Need to enable alignment faults separately of the MMU,
    // otherwise QEMU will hard reset the CPU
    ctrl.set(ARMControl::AlignmentFaults);

    // Flush all
    tlb_flush_all();
    dsb();
    isb();
    return Success;
}
#endif /* ARMV7 */

MemoryContext::Result ARMPaging::activate(bool initializeMMU)
{
    ARMControl ctrl;

    // Do we need to (re)enable the MMU?
    if (initializeMMU)
    {
        enableMMU();
    }
    // MMU already enabled, we only need to change first level table and flush caches.
    else
    {
#ifdef ARMV6
        mcr(p15, 0, 0, c7, c5,  0);    // flush entire instruction cache
        mcr(p15, 0, 0, c7, c10, 0);    // flush entire data cache
        mcr(p15, 0, 0, c7, c7,  0);    // flush entire cache
        mcr(p15, 0, 5, c7, c10, 0);    // data memory barrier
        mcr(p15, 0, 4, c7, c10, 0);    // memory sync barrier
#else
        m_cache.cleanInvalidate(Cache::Unified);
#endif /* ARMV6 */

        // Switch first page table and re-enable L1 caching
        ctrl.write(ARMControl::TranslationTable0, (((u32) m_firstTableAddr) |
            (1 << 3) | /* outer write-back, write-allocate */
            (1 << 6)   /* inner write-back, write-allocate */
        ));

        // Flush TLB caches
        tlb_flush_all();

        // Synchronize execution stream
        isb();
    }
    // Done. Update currently active context pointer
    m_current = this;
    return Success;
}

MemoryContext::Result ARMPaging::map(Address virt, Address phys, Memory::Access acc)
{
    // Modify page tables
    Result r = m_firstTable->map(virt, phys, acc, m_alloc);

    // Flush the TLB to refresh the mapping
    if (m_current == this)
        tlb_invalidate(virt);

    // Synchronize execution stream.
    isb();
    return r;
}

MemoryContext::Result ARMPaging::unmap(Address virt)
{
    // Clean the given data page in cache
    if (m_current == this)
        m_cache.cleanInvalidateAddress(Cache::Data, virt);

    // Modify page tables
    Result r = m_firstTable->unmap(virt, m_alloc);

    // Flush TLB to refresh the mapping
    if (m_current == this)
        tlb_invalidate(virt);

    // Synchronize execution stream
    isb();
    return r;
}

MemoryContext::Result ARMPaging::lookup(Address virt, Address *phys) const
{
    return m_firstTable->translate(virt, phys, m_alloc);
}

MemoryContext::Result ARMPaging::access(Address virt, Memory::Access *access) const
{
    return m_firstTable->access(virt, access, m_alloc);
}

MemoryContext::Result ARMPaging::releaseSection(const Memory::Range & range,
                                                const bool tablesOnly)
{
    return m_firstTable->releaseSection(range, m_alloc, tablesOnly);
}

MemoryContext::Result ARMPaging::releaseRange(Memory::Range *range)
{
    return m_firstTable->releaseRange(*range, m_alloc);
}
