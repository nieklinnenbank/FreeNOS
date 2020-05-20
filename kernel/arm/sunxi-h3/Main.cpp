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
#include <Macros.h>
#include <MemoryBlock.h>
#include <arm/ARMPaging.h>
#include <arm/ARMControl.h>
#include <arm/ARMCore.h>
#include <NS16550.h>
#include <DeviceLog.h>
#include <SunxiCoreServer.h>
#include "SunxiKernel.h"

extern Address __bootimg, __bss_start, __bss_end;

static u32 ALIGN(16 * 1024) SECTION(".data") tmpPageDir[4096];

extern C int kernel_main(void)
{
    // Invalidate all caches now
    Arch::Cache cache;
    cache.invalidate(Cache::Unified);

#ifdef ARMV7
    // Raise the SMP bit for ARMv7
    ARMControl ctrl;
    ctrl.set(ARMControl::SMPBit);
#endif

    // Setup memory map with kernel base physical memory address
    Arch::MemoryMap mem;
    Address kernelBaseAddr = RAM_ADDR;

    if (read_core_id() != 0) {
        CoreInfo tmpInfo;
        MemoryBlock::copy((void *)&tmpInfo,
            (void *)SunxiCoreServer::SecondaryCoreInfoAddress, sizeof(coreInfo));
        kernelBaseAddr = tmpInfo.kernel.phys;
    }

    // Prepare early page tables and re-map the temporary stack
    ARMPaging paging(&mem, (Address) &tmpPageDir, kernelBaseAddr);

    // Activate MMU
    paging.activate(true);

    // Fill coreInfo for boot core
    if (read_core_id() == 0)
    {
        BootImage *bootimage = (BootImage *) &__bootimg;
        MemoryBlock::set(&coreInfo, 0, sizeof(CoreInfo));
        coreInfo.bootImageAddress = (Address) (bootimage);
        coreInfo.bootImageSize    = bootimage->bootImageSize;
        coreInfo.kernel.phys      = RAM_ADDR;
        coreInfo.kernel.size      = MegaByte(4);
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
    MemoryBlock::set(&__bss_start, 0, &__bss_end - &__bss_start);

    // Initialize heap at the end of the kernel (and after embedded boot image)
    coreInfo.heapAddress = ((Address) &__bss_end) + coreInfo.bootImageSize;
    coreInfo.heapAddress &= PAGEMASK;
    coreInfo.heapAddress += PAGESIZE;
    coreInfo.heapSize = MegaByte(1);
    Kernel::heap(coreInfo.heapAddress, coreInfo.heapSize);

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
