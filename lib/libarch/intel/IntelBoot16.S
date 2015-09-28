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
 *
 */

#include <CoreInfo.h>
#include "IntelConstant.h"
#include "IntelMP.h"

/** Offset in physical memory where bootEntry16 must be loaded. */
#define LOADADDR 0xf000

/* Export symbols. */
.global bootEntry16

/*.section ".text"*/
.data
.code16

/*
 * 16-bit real mode entry point for each newly booted CPU.
 */
bootEntry16:
base = .

    /* Disable interrupts. */
    cli

    /* Setup segment registers. */
    mov %cs, %ax
    mov %ax, %ds

    /* Load the global descriptor table. */
    lgdtl gdt16_ptr - base

    /* Enable Protected mode. */
    movl %cr0, %eax
    orl  $(CR0_PE), %eax
    movl %eax, %cr0

    /* Reload data segment. */
    mov $KERNEL_DS_SEL, %ax
    mov %ax, %ds

    /* Reload code segment: jump to 32-bit entry point. */
    ljmpl $KERNEL_CS_SEL, $mode32 - base + LOADADDR

gdt16:
        .quad   0x0000000000000000 /* NULL descriptor. */
        .quad   0x00cf9a000000ffff /* Kernel CS. */
        .quad   0x00cf92000000ffff /* Kernel DS. */
        .quad   0x00cffa000000ffff /* User CS. */
        .quad   0x00cff2000000ffff /* User DS. */
        .quad   0x0000000000000000 /* TSS descriptor. */
gdt16_end:

gdt16_ptr:
        .word gdt16_end - gdt16
        .long gdt16 - base + LOADADDR
        .word 0

.data
.code32

mode32:

    /* Obtain memory base address: CoreInfo.memory.phys */
    movl $MPINFOADDR, %eax
    addl $12, %eax
    movl (%eax), %eax

    /* Point to IntelBoot32.S */
    movl $MPINFOADDR, %ecx
    addl $24, %ecx
    mov (%ecx), %ecx
    addl %eax, %ecx

    /* Set eax to coreInfo address */
    movl $MPINFOADDR, %eax

    /* Jump to IntelBoot32.S */
    jmp *%ecx
