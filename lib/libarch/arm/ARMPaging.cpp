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
#include "ARMCore.h"
#include "ARMControl.h"
#include "ARMPaging.h"

ARMPaging::ARMPaging(MemoryMap *map, SplitAllocator *alloc)
    : MemoryContext(map, alloc)
{
    // Allocate page directory from low physical memory.
    if (alloc->allocateLow(sizeof(ARMFirstTable),
                          &m_firstTableAddr,
                           sizeof(ARMFirstTable)) == Allocator::Success)
    {
        m_firstTable = (ARMFirstTable *) alloc->toVirtual(m_firstTableAddr);

        // Initialize the page directory
        MemoryBlock::set(m_firstTable, 0, sizeof(ARMFirstTable));

        // Map the kernel. The kernel has permanently mapped 1GB of
        // physical memory (i.e. the "low memory" in SplitAllocator). The low
        // memory starts at its physical base address offset (varies per core).
        m_firstTable->mapLarge( m_map->range(MemoryMap::KernelData), m_alloc );

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
    }
}

ARMPaging::~ARMPaging()
{
    // TODO: release physical memory for this context?
}

MemoryContext::Result ARMPaging::activate()
{
    ARMControl ctrl;

    // Do we need to (re)enable the MMU?
    if (!(ctrl.read(ARMControl::SystemControl) & ARMControl::MMUEnabled))
    {
        // Program first level table. Enable L2 cache for page walking.
        ctrl.write(ARMControl::TranslationTable0, ((u32) m_firstTableAddr | 1));
        ctrl.write(ARMControl::TranslationTable1,    0);
        ctrl.write(ARMControl::TranslationTableCtrl, 0);

        // Set Control flags
        ctrl.set(ARMControl::DomainClient);
        ctrl.set(ARMControl::ExtendedPaging);
        ctrl.set(ARMControl::DisablePageColoring);
        ctrl.set(ARMControl::AccessPermissions);
        ctrl.unset(ARMControl::BranchPrediction);

        // Flush TLB's (if any).
        tlb_flush_all();

        // Clean and invalidate both caches.
        // This writes back all data to RAM and empties the cache completely.
        ctrl.write(ARMControl::DataCacheClean, 0);
        ctrl.write(ARMControl::CacheClear, 0); // TODO: needed?
        ctrl.write(ARMControl::InstructionCacheClear, 0);
        ctrl.write(ARMControl::FlushPrefetchBuffer, 0);
        dsb();

        // Disable both caches.
        ctrl.unset(ARMControl::InstructionCache);
        ctrl.unset(ARMControl::DataCache);
        dsb();

        // Enable the MMU. This re-enables instruction and data cache too.
        // ctrl.set(ARMControl::MMUEnabled | ARMControl::InstructionCache | ARMControl::DataCache);
        // invalidate data cache and flush prefetch buffer
        //  asm volatile ("mcr p15, 0, %0, c7, c5,  4" :: "r" (0) : "memory");
        //asm volatile ("mcr p15, 0, %0, c7, c6,  0" :: "r" (0) : "memory");
        //
        // enable MMU, L1 cache and instruction cache, L2 cache, write buffer,
        //   branch prediction and extended page table on
        //unsigned mode;
        //asm volatile ("mrc p15,0,%0,c1,c0,0" : "=r" (mode));
        //mode |= 0x0480180D;
        //asm volatile ("mcr p15,0,%0,c1,c0,0" :: "r" (mode) : "memory");
        ctrl.set(ARMControl::MMUEnabled);
        NOTICE("MMUEnabled = " << (ctrl.read(ARMControl::SystemControl) & ARMControl::MMUEnabled));
        tlb_flush_all();

        // Reactivate both caches and branch prediction
        NOTICE("Going to enable the caches....");
        ctrl.set(ARMControl::InstructionCache);
        ctrl.set(ARMControl::DataCache);
        ctrl.set(ARMControl::BranchPrediction);
        NOTICE("Caches enabled");
    }
    // MMU already enabled, we only need to change first level table and flush caches.
    else
    {
        mcr(p15, 0, 0, c7, c5,  0);    // flush entire instruction cache
        mcr(p15, 0, 0, c7, c10, 0);    // flush entire data cache
        mcr(p15, 0, 0, c7, c7,  0);    // flush entire cache 
        mcr(p15, 0, 5, c7, c10, 0);    // data memory barrier 
        mcr(p15, 0, 4, c7, c10, 0);    // memory sync barrier 
        mcr(p15, 0, 0, c2, c0,  (m_firstTableAddr | 1)); // switch first page table. Enable L1 cache for page walking.
        mcr(p15, 0, 0, c8, c5,  0);    // flush instruction TLB 
        mcr(p15, 0, 0, c8, c6,  0);    // flush data TLB 
        mcr(p15, 0, 0, c8, c7,  0);    // flush unified TLB 
    }
    return Success;
}

MemoryContext::Result ARMPaging::map(Address virt, Address phys, Memory::Access acc)
{
    // On ARM, we invalidate the TLB entry, such that the MMU can insert a new TLB after the remap.
    // TODO: make conditional? (only if the context is currently activated)
    tlb_invalidate(virt);

    // Modify page tables
    Result r = m_firstTable->map(virt, phys, acc, m_alloc);

    ARMControl ctrl;
    ctrl.write(ARMControl::DataCacheClean, 0);
    tlb_flush_all();
    return r;
}

MemoryContext::Result ARMPaging::unmap(Address virt)
{
    // On ARM, we invalidate the TLB entry, such that the MMU can insert a new TLB after the remap.
    // TODO: make conditional? (only if the context is currently activated)
    tlb_invalidate(virt);

    // Modify page tables
    Result r = m_firstTable->unmap(virt, m_alloc);

    ARMControl ctrl;
    ctrl.write(ARMControl::DataCacheClean, 0);
    tlb_flush_all();
    return r;
}

MemoryContext::Result ARMPaging::lookup(Address virt, Address *phys)
{
    return m_firstTable->translate(virt, phys, m_alloc);
}

MemoryContext::Result ARMPaging::access(Address virt, Memory::Access *access)
{
    return m_firstTable->access(virt, access, m_alloc);
}
