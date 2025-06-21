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
#include <FreeNOS/Config.h>
#include <Log.h>
#include <ListIterator.h>
#include <SplitAllocator.h>
#include <BubbleAllocator.h>
#include <IntController.h>
#include <PoolAllocator.h>
#include <CoreInfo.h>
#include "Kernel.h"
#include "Memory.h"

Kernel::Kernel(CoreInfo *info)
    : WeakSingleton<Kernel>(this)
{
    // Output log banners on the boot core
    if (info->coreId == 0)
    {
        Log::instance()->append(BANNER);
        Log::instance()->append(COPYRIGHT "\r\n");
    }

    // Setup physical memory allocator
    const Arch::MemoryMap map;
    const Memory::Range kernelData = map.range(MemoryMap::KernelData);
    const Allocator::Range physRange = { info->memory.phys, info->memory.size, PAGESIZE };
    const Allocator::Range virtRange = { kernelData.virt, kernelData.size, PAGESIZE };
    m_alloc  = new SplitAllocator(physRange, virtRange, PAGESIZE);

    // Initialize other class members
    m_coreInfo   = info;
    m_intControl = ZERO;
    m_timer      = ZERO;

    // Print memory map
    NOTICE("kernel @ " << (void *) info->kernel.phys << ".." <<
                          (void *) (info->kernel.phys + info->kernel.size));
    NOTICE("bootImage @ " << (void *) info->bootImageAddress << ".." <<
                             (void *) (info->bootImageAddress + info->bootImageSize));
    NOTICE("heap @ " << (void *) (m_alloc->toPhysical(info->heapAddress)) << ".." <<
                        (void *) (m_alloc->toPhysical(info->heapAddress + info->heapSize)));

    if (info->coreChannelSize)
    {
        NOTICE("coreChannel @ " << (void *) info->coreChannelAddress << ".." <<
                                   (void *) (info->coreChannelAddress + info->coreChannelSize - 1));
    }

    // Verify coreInfo memory ranges
    assert(info->kernel.phys >= info->memory.phys);
    assert(info->bootImageAddress >= info->kernel.phys + info->kernel.size);
    assert(m_alloc->toPhysical(info->heapAddress) >= info->bootImageAddress + info->bootImageSize);

    // Only secondary cores need to allocate coreChannels
    if (info->coreId == 0)
    {
        assert(info->coreChannelAddress == ZERO);
        assert(info->coreChannelSize == ZERO);
    }
    else
    {
        assert(info->coreChannelAddress >= m_alloc->toPhysical(info->heapAddress + info->heapSize));
    }

    // Mark all kernel memory used
    for (Size i = 0; i < info->kernel.size; i += PAGESIZE)
        m_alloc->allocate(info->kernel.phys + i);

    // Mark BootImage memory used
    for (Size i = 0; i < m_coreInfo->bootImageSize; i += PAGESIZE)
        m_alloc->allocate(m_coreInfo->bootImageAddress + i);

    // Mark heap memory used
    for (Size i = 0; i < m_coreInfo->heapSize; i += PAGESIZE)
        m_alloc->allocate(m_alloc->toPhysical(m_coreInfo->heapAddress + i));

    // Reserve CoreChannel memory
    for (Size i = 0; i < m_coreInfo->coreChannelSize; i += PAGESIZE)
        m_alloc->allocate(m_coreInfo->coreChannelAddress + i);

}

Error Kernel::initializeHeap()
{
    // Calculate proper heap address: heap starts after the boot image.
    Size heapPhysical = coreInfo.bootImageAddress + coreInfo.bootImageSize;
    heapPhysical += PAGESIZE - (coreInfo.bootImageSize % PAGESIZE);

    // Heap must be a virtual address (see SplitAllocator)
    const Arch::MemoryMap map;
    const Memory::Range kernelData = map.range(MemoryMap::KernelData);
    coreInfo.heapAddress = heapPhysical - (coreInfo.memory.phys - kernelData.virt);
    coreInfo.heapSize    = MegaByte(1);

    // Prepare allocators
    Size metaData = sizeof(BubbleAllocator) + sizeof(PoolAllocator);
    Allocator *bubble, *pool;
    const Allocator::Range bubbleRange = { coreInfo.heapAddress + metaData,
                                           coreInfo.heapSize - metaData, sizeof(u32) };

    // Clear the heap first
    MemoryBlock::set((void *) coreInfo.heapAddress, 0, coreInfo.heapSize);

    // Setup the dynamic memory heap
    // placement new in coreInfo.headAddress
    bubble = new (coreInfo.heapAddress) BubbleAllocator(bubbleRange);
    pool   = new (coreInfo.heapAddress + sizeof(BubbleAllocator)) PoolAllocator(bubble);

    // Set default allocator
    Allocator::setDefault(pool);
    return 0;
}

SplitAllocator * Kernel::getAllocator()
{
    return m_alloc;
}

MemoryContext * Kernel::getMemoryContext()
{
#if 0
    return m_procs->current()->getMemoryContext();
#else
    return (MemoryContext *) NULL;
#endif
}

CoreInfo * Kernel::getCoreInfo()
{
    return m_coreInfo;
}

Kernel::Result Kernel::loadBootImage()
{
#if 0
    BootImageStorage bootImage((const BootImage *) (m_alloc->toVirtual(m_coreInfo->bootImageAddress)));

    // Initialize the BootImageStorage object
    const FileSystem::Result result = bootImage.initialize();
    if (result != FileSystem::Success)
    {
        FATAL("failed to initialize BootImageStorage: result = " << (int) result);
    }

    NOTICE("bootimage: " << (void *) m_coreInfo->bootImageAddress <<
           " (" << m_coreInfo->bootImageSize << " bytes)");

    // Read header
    const BootImage image = bootImage.bootImage();

    // Loop BootPrograms
    for (Size i = 0; i < image.symbolTableCount; i++)
    {
        BootSymbol program;

        const FileSystem::Result readResult = bootImage.read(
            image.symbolTableOffset + (sizeof(BootSymbol) * i),
            &program,
            sizeof(BootSymbol)
        );

        if (readResult != FileSystem::Success)
        {
            FATAL("failed to read BootSymbol: result = " << (int) readResult);
        }

        // Ignore non-BootProgram entries
        if (program.type != BootProgram && program.type != BootPrivProgram)
        {
            continue;
        }

        // Load the program
        const Result loadResult = loadBootProgram(bootImage, program);
        if (loadResult != Success)
        {
            FATAL("failed to load BootSymbol " << program.name << ": result = " << (int) loadResult);
            return IOError;
        }
    }
#endif
    return Success;
}

#if 0
Kernel::Result Kernel::loadBootProgram(const BootImageStorage &bootImage,
                                       const BootSymbol &program)
{
    const BootImage image = bootImage.bootImage();
    const Arch::MemoryMap map;

    // Only BootProgram entries allowed
    assert(program.type == BootProgram || program.type == BootPrivProgram);

    // Create process
    Process *proc = m_procs->create(program.entry, map, true, program.type == BootPrivProgram);
    if (!proc)
    {
        FATAL("failed to create boot program: " << program.name);
        return ProcessError;
    }

    // Obtain process memory
    MemoryContext *mem = proc->getMemoryContext();

    // Map program segment into it's virtual memory
    for (Size i = 0; i < program.segmentsCount; i++)
    {
        BootSegment segment;

        // Read the next BootSegment
        const FileSystem::Result result = bootImage.read(
            image.segmentsTableOffset + ((program.segmentsOffset + i) * sizeof(BootSegment)),
            &segment,
            sizeof(BootSegment)
        );

        if (result != FileSystem::Success)
        {
            FATAL("failed to read BootSegment for BootProgram " <<
                   program.name << ": result = " << (int) result);
            return ProcessError;
        }

        // Map memory
        Memory::Range range;
        range.phys = 0;
        range.virt = segment.virtualAddress;
        range.size = segment.size;
        range.access = Memory::User | Memory::Readable | Memory::Writable | Memory::Executable;

        const MemoryContext::Result mapResult = mem->mapRangeContiguous(&range);
        if (mapResult != MemoryContext::Success)
        {
            FATAL("failed to map BootSegment at " << (void *) segment.virtualAddress <<
                  " for BootProgram " << program.name << ": result = " << (int) mapResult);
            return ProcessError;
        }

        // Read from BootImage to physical memory of the program
        // This assumes direct access to that physical memory.
        const FileSystem::Result readResult = bootImage.read(
            segment.offset,
            (void *) m_alloc->toVirtual(range.phys),
            segment.size
        );

        if (readResult != FileSystem::Success)
        {
            FATAL("failed to read BootSegment data at " << (void *) segment.virtualAddress <<
                  " for BootProgram " << program.name << ": result = " << (int) readResult);
            return ProcessError;

        }
    }

    // Allocate page for program arguments
    Memory::Range argRange = map.range(MemoryMap::UserArgs);
    argRange.access = Memory::User | Memory::Readable | Memory::Writable;

    Allocator::Range alloc_args;
    alloc_args.address = 0;
    alloc_args.size = argRange.size;
    alloc_args.alignment = PAGESIZE;

    if (m_alloc->allocate(alloc_args) != Allocator::Success)
    {
        FATAL("failed to allocate program arguments page");
        return ProcessError;
    }
    argRange.phys = alloc_args.address;

    // Map program arguments in the process
    if (mem->mapRangeContiguous(&argRange) != MemoryContext::Success)
    {
        FATAL("failed to map program arguments page");
        return ProcessError;
    }

    // Copy program arguments
    char *args = (char *) m_alloc->toVirtual(argRange.phys);
    MemoryBlock::set(args, 0, argRange.size);
    MemoryBlock::copy(args, program.name, BOOTIMAGE_NAMELEN);

    // Done
    NOTICE("loaded: " << program.name);
    return Success;
}
#endif

int Kernel::run()
{
    NOTICE("");

    // Load boot image programs
    loadBootImage();

    while (true) {};

    // Never actually returns.
    return 0;
}
