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
static u64 ALIGN(16 * 1024) SECTION(".data") tmpPageDir[1024];

#include "uart.h"
#include "mbox.h"

#include <FreeNOS/System.h>
#include <MemoryBlock.h>
#include <arm64/ARM64Map.h>
#include <arm64/ARM64Paging.h>

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

    Arch::MemoryMap mem;
    for (int i = MemoryMap::KernelData; i <= MemoryMap::UserArgs; i++) {
        uart_puts("Memory Region ");
        uart_hex(i);
        uart_puts("\n");
        Memory::Range reg = mem.range(static_cast<MemoryMap::Region>(i));
        uart_puts("virt: ");
        uart_hex(reg.virt);
        uart_puts(", ");

        uart_puts("phys: ");
        uart_hex(reg.phys);
        uart_puts(", ");

        uart_puts("size: ");
        uart_hex(reg.size);
        uart_puts("\n");
    }

    uart_puts("MMU start\n");
    ARM64Paging paging(&mem, (Address) &tmpPageDir, RAM_ADDR);

    // Activate MMU
    paging.initialize();
    uart_puts("MMU initialized\n");
    paging.activate(true);
    uart_puts("MMU enabled\n");

#if 0
    u64 tmp = (u64)&tmpPageDir;
    tmp += 0xFFFFFFFFC0000000UL;

    volatile u64 *higher_address = (volatile u64 *)tmp;
    volatile u64 *lower_address = (volatile u64 *)&tmpPageDir;
    uart_puts("after MMU: [");
    uart_hex_u64(tmp);
    uart_puts("] = ");
    u64 test = *higher_address;
    uart_hex_u64(test);
    uart_puts("\nafter MMU: [");
    uart_hex_u64((u64)lower_address);
    uart_puts("] = ");
    uart_hex_u64(*lower_address);
    uart_puts("\n");
#endif

    // Clear BSS
    clearBSS();


    // echo everything back
    while(1) {
        uart_send(uart_getc());
    }
    return 0;
}
