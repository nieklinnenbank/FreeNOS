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


/* https://github.com/bztsrc/raspi3-tutorial/blob/master/03_uart1 */
extern Address __start, __end, __bootimg;

#include "uart.h"
#include "mbox.h"

#include <FreeNOS/System.h>
#include <FreeNOS/arm64/ARM64Kernel.h>
#include <MemoryBlock.h>
#include <arm64/ARM64Map.h>
#include <arm64/ARM64Paging.h>
#include <arm64/ARM64PageTable.h>
#include <arm64/ARM64Exception.h>
#include <arm64/ARM64Control.h>
#include <DeviceLog.h>
#include "CoreInfo.h"
#include "BootImage.h"
#include "RaspberryKernel.h"
#include "Support.h"
#include "PL011.h"

static char ALIGN(16 * 1024) SECTION(".data") tmpPageDir[sizeof(ARM64PageTable)];
static char ALIGN(16 * 1024) SECTION(".data") tmpPage1Dir[sizeof(ARM64PageTable)];
static char ALIGN(16 * 1024) SECTION(".data") tmpPage2Dir[sizeof(ARM64PageTable)];

extern C int kernel_main(void)
{
    // set up serial console
    uart_init();

    unsigned long el;
    asm volatile("mrs %0, CurrentEL" : "=r" (el) : : "cc");
    el = (el >> 2) & 3;

    uart_puts("EL: ");
    uart_hex(el);
    uart_puts("\n");

    uart_puts("Hello World!\n");
    
    // get the board's unique serial number with a mailbox call
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = MBOX_TAG_GETSERIAL;   // get serial number command
    mbox[3] = 8;                    // buffer size
    mbox[4] = 8;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("My serial number is: ");
        uart_hex(mbox[6]);
        uart_hex(mbox[5]);
        uart_puts("\n");
    } else {
        uart_puts("Unable to query serial!\n");
    }

    // Fill coreInfo
    BootImage *bootimage = (BootImage *) &__bootimg;
    MemoryBlock::set(&coreInfo, 0, sizeof(CoreInfo));
    coreInfo.bootImageAddress = (Address) (bootimage);
    coreInfo.bootImageSize    = bootimage->bootImageSize;
    coreInfo.kernel.phys      = (Address) &__start;
    coreInfo.kernel.size      = ((Address) &__end - (Address) &__start);
    coreInfo.memory.phys      = RAM_ADDR;
    coreInfo.memory.size      = RAM_SIZE;

    uart_puts("BootImage Start: ");
    uart_hex_u64(coreInfo.bootImageAddress);
    uart_puts("\nSize: ");
    uart_hex_u64(coreInfo.bootImageSize);
    uart_puts("\n");

    Arch::MemoryMap mem;
    uart_puts("MMU start\n");
    Address tables[2] = { (Address) tmpPage1Dir, (Address) tmpPage2Dir };
    ARM64Paging paging(&mem, (Address) &tmpPageDir, tables, RAM_ADDR);

    // Activate MMU
    paging.initialize();
    paging.activate(true);
    uart_puts("MMU enabled\n");

    // Clear BSS
    clearBSS();

    // Initialize heap
    Kernel::initializeHeap();

    // Run all constructors first
    constructors();

    // Open the serial console as default Log
    PL011 pl011(25);
    pl011.initialize();

    DeviceLog console(pl011);
    console.setMinimumLogLevel(Log::Notice);

    NOTICE("This is a bare metal");
    RaspberryKernel kernel(&coreInfo);

    NOTICE("Before accessing memory");
    u64 val = *(volatile u64 *)0xffffffffffffaaffull;

    NOTICE("After accessing memory");
    asm volatile ("svc #1" ::: "memory");

    // echo everything back
    while(1) {
        uart_send(uart_getc());
    }
    return 0;
}
