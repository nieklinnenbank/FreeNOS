/*
 * Copyright (C) 2009 Niek Linnenbank
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

#include "IntelConstant.h"
#include "IntelBoot.h" 

/** Initial stack size */
#define STACK_SIZE PAGESIZE

.global multibootEntry, multibootHeader

.section ".boot"

/*
 * Multiboot header, 32-bit aligned.
 */
.align 4

multibootHeader:
        .long  (MULTIBOOT_HEADER_MAGIC)
        .long  (MULTIBOOT_HEADER_FLAGS)
        .long -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
        .long  (multibootHeader)

.section ".text"
.code32

/**
 * Entry point.
 */
multibootEntry:

    /* Disable interrupts. */
    cli

    /* Setup temporary boot stack. */
    movl $(stack + STACK_SIZE), %esp
    movl %esp, %ebp

    /* Check multiboot magic */
    cmpl $MULTIBOOT_BOOTLOADER_MAGIC, %eax
    jnz halt

    /* Copy multiboot info to CoreInfo. */
    push %ebx
    call multibootToCoreInfo

    /* Jump to bootEntry32 */
    jmp bootEntry32

/**
 * Stop execution immediately.
 */
halt:
    cli
    hlt
    jmp halt

.section ".data"
.align PAGESIZE

stack: .fill STACK_SIZE, 1, 0
