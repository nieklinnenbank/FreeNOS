/*
 * Copyright (C) 2019 Niek Linnenbank
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

#include <FreeNOS/Config.h>
#include <FreeNOS/Support.h>
#include <FreeNOS/System.h>
#include <arm/ARMPaging.h>
#include <arm/ARMControl.h>
#include <arm/ARMCore.h>
#include <NS16550.h>
#include <DeviceLog.h>
#include <SunxiCoreServer.h>
#include <Macros.h>
#include <MemoryBlock.h>
#include <Memory.h>
#include "SunxiKernel.h"

extern Address __start, __end, __bootimg;

static u32 ALIGN(16 * 1024) SECTION(".data") tmpPageDir[4096];

extern C int kernel_main(void)
{
#ifdef ARMV7
    // Raise the SMP bit for ARMv7
    ARMControl ctrl;
    ctrl.set(ARMControl::SMPBit);
#endif

    if (read_core_id() == 0)
    {
        // Invalidate all caches now
        Arch::Cache cache;
        cache.invalidate(Cache::Unified);
    }

    // Setup memory map with the memory base physical memory address
    Arch::MemoryMap mem;
    Address memoryBaseAddr = RAM_ADDR;

    if (read_core_id() != 0) {
        CoreInfo tmpInfo;
        MemoryBlock::copy((void *)&tmpInfo,
            (void *)SunxiCoreServer::SecondaryCoreInfoAddress, sizeof(coreInfo));
        memoryBaseAddr = tmpInfo.memory.phys;
    }

    // Prepare early page tables and re-map the temporary stack
    ARMPaging paging(&mem, (Address) &tmpPageDir, memoryBaseAddr);

    // Activate MMU
    paging.initialize();
    paging.activate(true);

    // Fill coreInfo for boot core
    if (read_core_id() == 0)
    {
        BootImage *bootimage = (BootImage *) &__bootimg;
        MemoryBlock::set(&coreInfo, 0, sizeof(CoreInfo));
        coreInfo.bootImageAddress = (Address) (bootimage);
        coreInfo.bootImageSize    = bootimage->bootImageSize;
        coreInfo.kernel.phys      = (Address) &__start;
        coreInfo.kernel.size      = ((Address) &__end - (Address) &__start);
        coreInfo.memory.phys      = RAM_ADDR;
        coreInfo.memory.size      = RAM_SIZE;
    }
    // Copy CoreInfo prepared by the CoreServer
    else
    {
        MemoryBlock::copy((void *)&coreInfo,
            (void *)SunxiCoreServer::SecondaryCoreInfoAddress, sizeof(coreInfo));
    }

    // Clear BSS
    clearBSS();

    // Initialize heap
    Kernel::initializeHeap();

    // Run all constructors first
    constructors();

    // Open serial console as default Log
    NS16550 *uart = new NS16550(UART0_IRQ);
    uart->initialize();
    DeviceLog *console = new DeviceLog(*uart);

    // Only the boot core outputs notifications
    if (read_core_id() == 0)
        console->setMinimumLogLevel(Log::Notice);
    else
        console->setMinimumLogLevel(Log::Warning);

    // Create the kernel
    SunxiKernel *kernel = new SunxiKernel(&coreInfo);

    // Run the kernel
    return kernel->run();
}
