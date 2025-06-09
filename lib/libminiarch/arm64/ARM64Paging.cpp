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
#if 0
#include "CoreInfo.h"
#include "ARM64Core.h"
#include "ARM64Control.h"
#endif
#include "ARM64Paging.h"
#include "ARM64PageTable.h"
#include "ARM64Control.h"

extern C void uart_puts(char *s);

ARM64Paging::ARM64Paging(MemoryMap *map, SplitAllocator *alloc)
    : MemoryContext(map, alloc)
    , m_firstTable(0)
    , m_firstTableAddr(0)
#if 0
    , m_kernelBaseAddr(coreInfo.memory.phys)
#endif
{
}

ARM64Paging::~ARM64Paging()
{
    if (m_firstTableAddr != 0)
    {
        for (Size i = 0; i < sizeof(ARM64PageTable); i += PAGESIZE)
            m_alloc->release(m_firstTableAddr + i);
    }
}

ARM64Paging::ARM64Paging(MemoryMap *map,
                     Address firstTableAddress,
                     Address kernelBaseAddress)
    : MemoryContext(map, ZERO)
    , m_firstTable((ARM64PageTable *) firstTableAddress)
    , m_firstTableAddr(firstTableAddress)
    , m_kernelBaseAddr(kernelBaseAddress)
{
}

MemoryContext::Result ARM64Paging::initialize()
{
    // Allocate first page table if needed
    // Which means at early boot stage, we use preallocated page table
    // TODO: 
    // Modify that if kernel address starts from high address, since level 2
    // only supports 1G memory range
    u8 level = 1;
    if (m_firstTable != 0)
        level = 2; /* start from level 2 */
    else {
        Allocator::Range phys, virt;
        phys.address = 0;
        phys.size = sizeof(ARM64PageTable);
        phys.alignment = sizeof(ARM64PageTable);

        // Allocate page directory
        if (m_alloc->allocate(phys, virt) != Allocator::Success)
        {
            return MemoryContext::OutOfMemory;
        }

        m_firstTable = (ARM64PageTable *) virt.address;
        m_firstTableAddr = phys.address;
    }

    // Initialize the page directory
    MemoryBlock::set(m_firstTable, 0, sizeof(ARM64PageTable));
    m_firstTable->m_level = level;

    // Map the kernel. The kernel has permanently mapped 1GB of
    // physical memory. This 1GiB memory region starts at its physical
    // base address offset which varies per core.
    Memory::Range kernelRange = m_map->range(MemoryMap::KernelData);
    kernelRange.phys = m_kernelBaseAddr;
    m_firstTable->mapBlock(kernelRange, m_alloc);

#if 0
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
#endif

    // Unmap I/O zone
    for (Size i = 0; i < IO_SIZE; i += MegaByte(2))
        m_firstTable->unmap(IO_BASE + i, m_alloc);

    // Map the I/O zone as Device / Uncached memory.
    Memory::Range io;
    io.phys = IO_BASE;
    io.virt = IO_BASE;
    io.size = IO_SIZE;
    io.access = Memory::Readable | Memory::Writable | Memory::Device;
    m_firstTable->mapBlock(io, m_alloc);

    return MemoryContext::Success;
}

//TODO: enable MMU
MemoryContext::Result ARM64Paging::enableMMU()
{
    u64 r, b, level;

    if (m_firstTable->m_level == 2)
        level = 34LL;
    else
        level = 25LL;

    r = ARM64Control::read(ARM64Control::MemoryModelFeature);
    b = PA_RANGE(r);
    if(TGRAN4(r)/*4k*/ || b<1/*36 bits*/) {
        uart_puts("ERROR: 4k granule or 36 bit address space not supported\n");
        return MemoryContext::InvaildArgs;
    }

    // first, set Memory Attributes array, indexed by PT_MEM, PT_DEV, PT_NC in our example
    r = MAIR_FIELD(MEM_ATTR_NORMAL, 0) |
        MAIR_FIELD(MEM_ATTR_DEV, 1) |
        MAIR_FIELD(MEM_ATTR_NC, 2);

    ARM64Control::write(ARM64Control::MemoryAttrIndirection, r);

    // next, specify mapping characteristics in translate control register
    r=  (0b00LL << 37) | // TBI=0, no tagging
        (b << 32) |      // IPS=autodetected
        (0b10LL << 30) | // TG1=4k
        (0b11LL << 28) | // SH1=3 inner
        (0b01LL << 26) | // ORGN1=1 write back
        (0b01LL << 24) | // IRGN1=1 write back
        (0b0LL  << 23) | // EPD1 enable higher half
        (level  << 16) | // T1SZ=25, 3 levels (512G); 34, 2 levels (1G) FIXME: is it necessary?
        (0b00LL << 14) | // TG0=4k
        (0b11LL << 12) | // SH0=3 inner
        (0b01LL << 10) | // ORGN0=1 write back
        (0b01LL << 8) |  // IRGN0=1 write back
        (0b0LL  << 7) |  // EPD0 enable lower half
        (level  << 0);   // T0SZ=25, 3 levels (512G); 34, 2 levels (1G)
    ARM64Control::write(ARM64Control::TranslationTableCtrl, r);
    

    // tell the MMU where our translation tables are. TTBR_CNP bit not documented, but required
    u64 tbl = (unsigned long)&m_firstTable->m_tables[0];
    tbl += 0x1UL;

    ARM64Control::write(ARM64Control::TranslationTable0, tbl);
    ARM64Control::write(ARM64Control::TranslationTable1, tbl);

    // finally, toggle some bits in system control register to enable page translation
    dsb(ish);
    isb();
    r = ARM64Control::read(ARM64Control::SystemControl);
    r |= 0xC00800;     // set mandatory reserved bits
    r &= ~(SystemControlFlags::EE |               // clear EE, little endian translation tables
          SystemControlFlags::E0E |              // clear E0E
          SystemControlFlags::WXN |              // clear WXN
          SystemControlFlags::InstrCache |       // clear I, no instruction cache
          SystemControlFlags::SA0 |              // clear SA0
          SystemControlFlags::SA |               // clear SA
          SystemControlFlags::Stage1Cache |      // clear C, no cache at all
          SystemControlFlags::AlignCheckEnabled);// clear A, no aligment check
    r |= SystemControlFlags::MMUEnabled;        // set M, enable MMU
    ARM64Control::write(ARM64Control::SystemControl, r);
    isb();

    return MemoryContext::Success;
}
MemoryContext::Result ARM64Paging::activate(bool initializeMMU)
{
    // Do we need to (re)enable the MMU?
    if (initializeMMU)
    {
        enableMMU();
    }
    return MemoryContext::Success;
}

MemoryContext::Result ARM64Paging::map(Address virt, Address phys, Memory::Access acc)
{
#if 0
    // Modify page tables
    Result r = m_firstTable->map(virt, phys, acc, m_alloc);

    // Flush the TLB to refresh the mapping
    if (m_current == this)
        tlb_invalidate(virt);

    // Synchronize execution stream.
    isb();
    return r;
#else
    return MemoryContext::Success;
#endif
}

MemoryContext::Result ARM64Paging::unmap(Address virt)
{
#if 0
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
#else
    // Modify page tables
    Result r = m_firstTable->unmap(virt, m_alloc);
    return r;
#endif
}

MemoryContext::Result ARM64Paging::lookup(Address virt, Address *phys) const
{
    return m_firstTable->translate(virt, phys, m_alloc);
}

MemoryContext::Result ARM64Paging::access(Address virt, Memory::Access *access) const
{
    return m_firstTable->access(virt, access, m_alloc);
}

MemoryContext::Result ARM64Paging::releaseSection(const Memory::Range & range,
                                                const bool tablesOnly)
{
    return m_firstTable->releaseBlock(range, m_alloc, tablesOnly);
}

MemoryContext::Result ARM64Paging::releaseRange(Memory::Range *range)
{
    return m_firstTable->releaseRange(*range, m_alloc);
}
