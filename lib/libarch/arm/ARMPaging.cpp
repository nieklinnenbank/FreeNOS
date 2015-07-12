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

#include <FreeNOS/API.h>
#include <MemoryBlock.h>
#include <Types.h>
#include <BitAllocator.h>
#include <Log.h>
#include "ARMCore.h"
#include "ARMPaging.h"
#include "ARMControl.h"

/**
 * Temporary definitions. Need to be put in a separate class
 *
 * These bits are set in APX, S, AP.
 *
 * @see ARM Architecture Reference Manual, page 731.
 */
#define PAGE1_NONE       0
#define PAGE1_PRESENT    1

/*
 *  nG: bits 11
 *   S: bits 10
 * APX: bits 9
 * TEX: bits 6, 7, 8
 *  AP: bits 4, 5
 *   C: bits 3 
 *   B: bits 2
 *  XN: bits 0
 *
 * @see ARM Architecture Reference Manual, page 709.
 */
#define PAGE2_NONE      (0)
#define PAGE2_PRESENT   ((1 << 1) | PAGE2_SHARED | PAGE2_TEX) /*| PAGE2_CACHED | PAGE2_BUFFERED | PAGE2_TEX */
#define PAGE2_TEX       (1 << 6)
#define PAGE2_NOEXEC    (1 << 0)
#define PAGE2_CACHED    (1 << 3)
#define PAGE2_BUFFERED  (1 << 2)
#define PAGE2_DEVICE    ((1 << 7))
#define PAGE2_SHARED    (1 << 10)

/* System Read-only Flag */
#define PAGE2_APX       (1 << 9)

/* User permissions */
#define PAGE2_AP_SYSONLY   (1 << 4)
#define PAGE2_AP_USERREAD  (1 << 5)
#define PAGE2_AP_FULL     ((1 << 4) | (1 << 5))

/*
 * Second level page tables are mapped in the PageTables memory region.
 * First level page table is mapped in the KernelPrivate memory region.
 */

ARMPaging::ARMPaging(Address pageDirectory, BitAllocator *phys)
    : Memory(pageDirectory, phys)
{
    ARMControl ctrl;
    Memory::Range pageTabRange = range(PageTables);

    // Default to the local page directory
    m_mmuEnabled        = true;
    m_pageTableBase     = range(PageTables).virt;
    m_localDirectory    = (Address *) PAGEDIR_LOCAL;
    m_pageDirectory     = m_localDirectory;
    m_pageDirectoryPhys = 0;

    if (pageDirectory)
    {
        // Map first level page table
        Address dir = findFree(KiloByte(16), KernelPrivate);
        for (Size i = 0; i < PAGEDIR_SIZE; i += PAGESIZE)
            map(pageDirectory + i, dir + i, Readable | Writable);

        m_pageDirectoryPhys = pageDirectory;
        m_pageDirectory     = (Address *) dir;

        // Map second level page tables
        for (Size i = 0; i < pageTabRange.size; i += MegaByte(4))
        {
            Address base = pageTabRange.virt + i;

            if (!(m_localDirectory[ DIRENTRY(base) ] & PAGE1_PRESENT))
            {
                for (Size j = 0; j < 4; j++)
                    m_localDirectory[ DIRENTRY(base) + j ] = m_pageDirectory[ DIRENTRY(pageTabRange.virt) + j];
                m_pageTableBase = base;
                break;
            }
        }
        tlb_flush_all();
    }
}

ARMPaging::~ARMPaging()
{
    if (m_pageDirectory != m_localDirectory)
    {
        // Save local members
        Address  savedBase = m_pageTableBase;
        Address *savedDir  = m_pageDirectory;
        m_pageTableBase    = range(PageTables).virt;
        m_pageDirectory    = m_localDirectory;

        // Unmap first level table
        for (Size i = 0; i < PAGEDIR_SIZE; i += PAGESIZE)
            unmap(((Address) savedDir) + i);

        // Unmap second level tables
        for (Size i = 0; i < 4; i++)
            m_localDirectory[ DIRENTRY(savedBase) + i ] = PAGE1_NONE;

        tlb_flush_all();
    }
}

Memory::Range ARMPaging::range(Memory::Region region)
{
    Memory::Range r;

    // Initialize unused fields
    r.phys   = 0;
    r.access = None;

    // Fill region virtual range
    switch (region)
    {
        case KernelData:    r.virt = 0x0;        r.size = MegaByte(3);   break;
        case KernelHeap:    r.virt = 0x00300000; r.size = MegaByte(1);   break;
        case KernelStack:   r.virt = 0x08400000; r.size = KiloByte(4);   break;
        case PageTables:    r.virt = 0x00400000; r.size = MegaByte(64);  break;
        /* PAGEDIR_LOCAL is at 0x440000 */
        case KernelPrivate: r.virt = 0x04404000; r.size = MegaByte(64);  break;
        case UserData:      r.virt = 0x80000000; r.size = MegaByte(256); break;
        case UserHeap:      r.virt = 0xb0000000; r.size = MegaByte(256); break;
        case UserStack:     r.virt = 0xc0000000; r.size = KiloByte(4);   break;
        case UserPrivate:   r.virt = 0xa0000000; r.size = MegaByte(256); break;
        case UserShared:    r.virt = 0xd0000000; r.size = MegaByte(256); break;
    }
    return r;
}

Memory::Result ARMPaging::create()
{
    Memory::Range r;
    Size size = PAGESIZE;
    Address secondPageTabs;
    Memory::Range pageTabRange = range(PageTables);

    // Clear the first level page table
    MemoryBlock::set(m_pageDirectory, PAGE1_NONE, PAGEDIR_SIZE);

    // Inherit the kernel data and heap into the new context
    r = range(KernelData);
    for (Size i = 0; i < r.size; i += PAGETAB_SPAN)
        m_pageDirectory[DIRENTRY(r.virt+i)] = m_localDirectory[DIRENTRY(r.virt+i)];

    r = range(KernelHeap);
    for (Size i = 0; i < r.size; i += PAGETAB_SPAN)
        m_pageDirectory[DIRENTRY(r.virt+i)] = m_localDirectory[DIRENTRY(r.virt+i)];

    tlb_flush_all();

    // Allocate second level page tables, which map the page tables for this context itself
    m_phys->allocate(&size, &secondPageTabs);

    // Save local members
    Address  savedBase = m_pageTableBase;
    Address *savedDir  = m_pageDirectory;
    m_pageTableBase    = range(PageTables).virt;
    m_pageDirectory    = m_localDirectory;

    // Fill second level page tables (requires mapping them into the local address space)
    Address v = findFree(size, KernelPrivate);
    map(secondPageTabs, v, Readable|Writable);
    MemoryBlock::set((void *)v, PAGE2_NONE, size);

    // Note that the 2nd page tables that map all 2nd page tables also needs to be modified,
    // for when adding more page tables). This self-mapping should start at 0x401000.
    // See the getPageTable(tableSelfAddr) trick in map().
    ((Address *) v)[0] = ((Address *)(m_pageTableBase+0x1000))[0];
    ((Address *) v)[1] = secondPageTabs | PAGE2_PRESENT | PAGE2_AP_SYSONLY;
    unmap(v);

    // Restore local members
    m_pageTableBase = savedBase;
    m_pageDirectory = savedDir;

    for (Size i = 0; i < 4; i++)
        m_pageDirectory[/* DIRENTRY(pageTabRange.virt)?? */ 4+i] = (secondPageTabs + (i*PAGETAB_SIZE)) | PAGE1_PRESENT;

    // Map second level page tables in local address space
    for (Size i = 0; i < pageTabRange.size; i += MegaByte(4))
    {
        Address base = pageTabRange.virt + i;

        if (!(m_localDirectory[ DIRENTRY(base) ] & PAGE1_PRESENT))
        {
            for (Size j = 0; j < 4; j++)
                m_localDirectory[ DIRENTRY(base) + j ] = m_pageDirectory[ DIRENTRY(pageTabRange.virt) + j];
            m_pageTableBase = base;
            break;
        }
    }
    tlb_flush_all();

    // Map first level page table
    for (Size i = 0; i < PAGEDIR_SIZE; i += PAGESIZE)
        map((m_pageDirectoryPhys) + i, PAGEDIR_LOCAL + i, Readable | Writable);

    // TODO: add the memory mapped I/O zone (identity mapping)
    for (Size i = 0; i < 0x1000000; i += PAGESIZE)
        map(0x20000000+i, 0x20000000+i, Readable | Writable | IO | User);

    return Success;
}



// TODO: put this in separate ARM memory management module instead?
Memory::Result ARMPaging::initialize()
{
    ARMControl ctrl;
    Size size = KiloByte(16);
    Address secondPageTabs;
    Address *savedPageDirectory = m_pageDirectory;

    // Allocate first level page table
    m_mmuEnabled = false;
    m_phys->allocate(
        &size,               // 16KB size. We use it for both the OS and user for now
        (Address *) &m_pageDirectory, // first level page table
        KiloByte(16)         // first level page table must be 16KB aligned
    );

    // Initialize first level page table
    for (Size i = 0; i < PAGEDIR_MAX; i++)
        *(m_pageDirectory + i) = PAGE1_NONE;

    // Map second level page tables, which map the page tables for this context itself
    size = PAGESIZE;
    m_phys->allocate(&size, &secondPageTabs);
    MemoryBlock::set((void *) secondPageTabs, PAGE2_NONE, size);

    for (Size i = 0; i < 4; i++)
        m_pageDirectory[4+i] = (secondPageTabs + (i*PAGETAB_SIZE)) | PAGE1_PRESENT;

    // Note that the 2nd page tables that map all 2nd page tables also needs to be modified,
    // for when adding more page tables). This self-mapping should start at 0x401000.
    // See the getPageTable(tableSelfAddr) trick in map().
    ((Address *)secondPageTabs)[1] = secondPageTabs | PAGE2_PRESENT | PAGE2_AP_SYSONLY;

    // Map first level page table
    for (Size i = 0; i < PAGEDIR_SIZE; i += PAGESIZE)
        map(((Address)m_pageDirectory) + i, PAGEDIR_LOCAL + i, Readable | Writable);

    // Map kernel code and data
    Range r = range(KernelData);
    for (Size i = r.virt; i < r.virt+r.size; i += PAGESIZE)
        map(i, i, Readable | Writable | Executable);

    // Map kernel heap
    r = range(KernelHeap);
    for (Size i = r.virt; i < r.virt+r.size; i += PAGESIZE)
        map(i, i, Readable | Writable);

    // TODO: add the memory mapped I/O zone (identity mapping)
    for (Size i = 0; i < 0x1000000; i += PAGESIZE)
        map(0x20000000+i, 0x20000000+i, Readable | Writable | IO | User);

    // Program first level tables
    ctrl.write(ARMControl::TranslationTable0, ((u32) m_pageDirectory) | 3); // inner cache, outer non-cache, shareable. Needed???
    ctrl.write(ARMControl::TranslationTable1,    0);
    ctrl.write(ARMControl::TranslationTableCtrl, 0);

    // Set Control flags
    ctrl.set(ARMControl::DomainClient);
    ctrl.set(ARMControl::ExtendedPaging);
    ctrl.set(ARMControl::DisablePageColoring);

    // Flush TLB's
    tlb_flush_all();

    // Disable and invalidate instruction cache
#if 0
    ctrl.unset(ARMControl::InstructionCache);
    ctrl.unset(ARMControl::DataCache);
#endif
    ctrl.write(ARMControl::InstructionCacheClear, 0);
    ctrl.write(ARMControl::CacheClear, 0);

    // Place memory barriers
    dsb();
    //dmb();

    // Enable the MMU. This re-enables instruction and data cache too.
    // ctrl.set(ARMControl::MMUEnabled | ARMControl::InstructionCache | ARMControl::DataCache);
  // invalidate data cache and flush prefetch buffer
//  asm volatile ("mcr p15, 0, %0, c7, c5,  4" :: "r" (0) : "memory");
  //asm volatile ("mcr p15, 0, %0, c7, c6,  0" :: "r" (0) : "memory");

  // enable MMU, L1 cache and instruction cache, L2 cache, write buffer,
  //   branch prediction and extended page table on
  unsigned mode;
  asm volatile ("mrc p15,0,%0,c1,c0,0" : "=r" (mode));
  mode |= 0x0480180D;
  asm volatile ("mcr p15,0,%0,c1,c0,0" :: "r" (mode) : "memory");

    // Reactivate caches
    //DEBUG("restore caches");
    //ctrl.set(ARMControl::InstructionCache);
    //ctrl.set(ARMControl::DataCache);
    NOTICE("MMUEnabled = " << (ctrl.read(ARMControl::SystemControl) & ARMControl::MMUEnabled));

    // Restore members
    m_pageDirectory = savedPageDirectory;
    m_mmuEnabled = true;
    return Success;
}

Address * ARMPaging::getPageTable(Address virt, bool useBase)
{
    if (!useBase && !(m_pageDirectory[ DIRENTRY(virt) ] & PAGE1_PRESENT))
        return (Address *) ZERO;
    else if (m_mmuEnabled || useBase)
        return ((Address *) m_pageTableBase) + (((virt & PAGEMASK) >> DIRSHIFT) * PAGETAB_MAX);
    else
        return (Address *) (m_pageDirectory[ DIRENTRY(virt) ] & ~0xff);
}

Memory::Result ARMPaging::map(Address phys, Address virt, Access acc)
{
    Address *pageTable = getPageTable(virt);
    Size size = PAGESIZE;

    // Does the page table exist?
    if (!pageTable)
    {
        // Allocate a new page table buffer
        Address table;
        m_phys->allocate(&size, &table);

        // ARM has 1KB page tables, and 4KB pages. Thus we need
        // to (de)allocate per 4 page tables on ARM.
        Size idx = DIRENTRY(virt) - (DIRENTRY(virt) % 4);

        // Insert in the first-level table
        for (Size i = 0; i < 4; i++)
            m_pageDirectory[ idx + i ] = (table + (i*PAGETAB_SIZE)) | PAGE1_PRESENT;

        // Map the page table itself inside the m_pageTableBase range.
        Address tableSelfAddr = range(PageTables).virt + (idx * PAGETAB_SIZE);
        Address *tableSelfMap = getPageTable(tableSelfAddr);
        tableSelfMap[ TABENTRY(tableSelfAddr) ] = table | PAGE2_PRESENT | PAGE2_AP_SYSONLY;

        // Flush and clear the four page tables
        for (Size i = 0; i < 4; i++)
        {
            pageTable = getPageTable((virt - (virt % (PAGESIZE*4))));
            tlb_flush(pageTable);
            MemoryBlock::set(pageTable, PAGE2_NONE, PAGETAB_SIZE);
        }
        pageTable = getPageTable(virt);
    }
    // Map the physical page to a virtual address
    tlb_flush(pageTable);
    pageTable[ TABENTRY(virt) ] = (phys & PAGEMASK) | PAGE2_PRESENT | flags(acc);
    tlb_flush(virt);
    return Success;
}

Address ARMPaging::lookup(Address virt)
{
    Address *pageTable = getPageTable(virt);

    if (!pageTable)
        return ZERO;
    else
        return pageTable[ TABENTRY(virt) ] & PAGEMASK;
}

u32 ARMPaging::flags(Access acc)
{
    u32 f = 0;

    if (acc & Memory::Present)       f |= PAGE2_PRESENT;
    if (!(acc & Memory::Executable)) f |= PAGE2_NOEXEC;
    if (acc & Memory::IO)            f |= PAGE2_DEVICE;

    if (acc & Memory::User)
    {
        if (acc & Memory::Writable)
            f |= PAGE2_AP_FULL;
        else
            f |= PAGE2_AP_USERREAD;
    }
    else
    {
        f |= PAGE2_AP_SYSONLY;

        if (!(acc & Memory::Writable))
            f |= PAGE2_APX;
    }
    return f;
}

Memory::Access ARMPaging::access(Address virt)
{
    Address *pageTable = getPageTable(virt);
    Access acc = None;

    if (pageTable)
    {
        Address entry = pageTable[TABENTRY(virt)];

        if (entry & PAGE2_PRESENT)
            acc |= Memory::Present | Memory::Readable;

        if (entry & PAGE2_AP_USERREAD)
            acc |= Memory::User;

        if (entry & PAGE2_AP_FULL)
            acc |= Memory::User | Memory::Writable;
    }
    return acc;
}

Memory::Result ARMPaging::unmap(Address virt)
{
    Address *pageTable = getPageTable(virt);
    
    // Remove the mapping and flush TLB cache
    pageTable[ TABENTRY(virt) ] = 0;
    tlb_flush(virt);
    return Success;
}

Memory::Result ARMPaging::release(Address virt)
{
    Address physical = lookup(virt);

    if (physical)
        m_phys->release(physical);

    unmap(virt);
    return Success;
}

Memory::Result ARMPaging::releaseRegion(Memory::Region region)
{
    Range r = range(region);

    // Release physical pages of the whole region
    for (Size i = 0; i < r.size; i += PAGESIZE)
    {
        Address *table = getPageTable(r.virt + i);

        if (table)
            m_phys->release(table[TABENTRY(r.virt + i)] & PAGEMASK);
    }
    return Success;
}

Address ARMPaging::findFree(Size size, Memory::Region region)
{
    Range r = range(region);
    Size currentSize = 0;
    Address addr = r.virt, currentAddr = 0;

    while (addr < r.virt+r.size && currentSize < size)
    {
        Address *table = getPageTable(addr);

        // Does the page table exist at all?
        if (!table)
        {
            if (!currentAddr)
                currentAddr = addr;
            currentSize += PAGESIZE * PAGETAB_MAX;
            addr += PAGESIZE * PAGETAB_MAX;
        }
        // Is this virtual address unused?
        else if (!(table[TABENTRY(addr)] & PAGE2_PRESENT))
        {
            if (!currentAddr)
                currentAddr = addr;
            currentSize += PAGESIZE; addr += PAGESIZE;
        }
        // Used. Reset the search
        else
        {
            currentSize = 0; currentAddr = 0; addr += PAGESIZE;
        }
    }
    return currentAddr;
}
