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

#include <intel/IntelConstant.h>
#include <intel/IntelBoot.h>

/* Constants */
#define PAGE_PRESENT    1
#define PAGE_WRITE      2
#define PAGE_4MB        (1 << 7)
#define PAGE_4MB_SHIFT  22
#define KERNEL_LOWMEM   ((1024 * 1024 * 1024) - (1024 * 1024 * 128))
#define STACK_SIZE 0x4000

/*
 * TODO: move interrupt handling and IDT setup to libarch's IntelCore code.
 * see Desktop/scratch1
 */

/**
 * Generates interrupt handlers.
 */
.macro interruptHandler vnum, verr
i\vnum:
.if !\verr
    pushl $0
.endif
    pushl $\vnum
    jmp interruptHandler
.endm

/**
 * Fills in IDT entries.
 */
.macro idtEntry vnum, vtype
    mov $8, %eax
    mov $\vnum, %ebx
    imul %ebx
    add $idt, %eax
    mov $i\vnum, %ebx
    movw %bx, (%eax)             /* Offset low */
    shrl $16, %ebx
    movw %bx, 6(%eax)            /* Offset high */
    movw $KERNEL_CS_SEL, 2(%eax) /* Kernel CS */
    movb $0, 4(%eax)             /* Zeroes */
    movb $\vtype, 5(%eax)        /* Present, 32 bits, 01110 */
.endm

.global bootEntry32, gdt, kernelPageDir, kernelPageTab, kernelTss, kernelioBitMap

.section ".text"
.code32

/**
 * Entry point.
 */
bootEntry32:

    /* Disable interrupts. */
    cli

    /* Setup temporary boot stack. */
    movl $(stack + STACK_SIZE), %esp
    movl %esp, %ebp

    /* Load GDT. */
    lgdt gdtPtr

    /* Fill in IDT entries 0 - 16, and 32 - 47. */
    idtEntry 0, 0x8e
    idtEntry 1, 0x8e
    idtEntry 2, 0x8e
    idtEntry 3, 0x8e
    idtEntry 4, 0x8e
    idtEntry 5, 0x8e
    idtEntry 6, 0x8e
    idtEntry 7, 0x8e
    idtEntry 8, 0x8e
    idtEntry 9, 0x8e
    idtEntry 10, 0x8e
    idtEntry 11, 0x8e
    idtEntry 12, 0x8e
    idtEntry 13, 0x8e
    idtEntry 14, 0x8e
    idtEntry 15, 0x8e
    idtEntry 16, 0x8e
    idtEntry 32, 0x8e
    idtEntry 33, 0x8e
    idtEntry 34, 0x8e
    idtEntry 35, 0x8e
    idtEntry 36, 0x8e
    idtEntry 37, 0x8e
    idtEntry 38, 0x8e
    idtEntry 39, 0x8e
    idtEntry 40, 0x8e
    idtEntry 41, 0x8e
    idtEntry 42, 0x8e
    idtEntry 43, 0x8e
    idtEntry 44, 0x8e
    idtEntry 45, 0x8e
    idtEntry 46, 0x8e
    idtEntry 47, 0x8e
    idtEntry 48, 0x8e
    idtEntry 0x90, 0xee

    /* Load IDT. */
    lidt idtPtr

    /* Setup segments. */
    movl $KERNEL_DS_SEL, %eax
    movl %eax, %ds
    movl %eax, %es
    movl %eax, %ss

setupKernelDir:

    /* map 1GB for the kernel (incl 128MB private mappings) */
    movl $kernelPageDir, %eax /* eax: pagedir pointer */
    xorl %ecx, %ecx           /* ecx: address to map */

1:
    movl %ecx, %ebx           /* ebx: pagedir entry */
    orl  $(PAGE_PRESENT | PAGE_WRITE | PAGE_4MB), %ebx
    movl %ebx, (%eax)
    addl $4, %eax
    addl $4194304, %ecx
    cmpl $KERNEL_LOWMEM, %ecx
    jnz 1b

    /* Clear the rest of the page directory */
    movl $kernelPageDir, %ecx
    addl $4096, %ecx
    xorl %ebx, %ebx
2:
    movl %ebx, (%eax)
    addl $4, %eax
    cmpl %ecx, %eax
    jnz 2b

    /* Enable timestamp counter and page size extension. */
    movl %cr4, %eax
    andl  $(~CR4_TSD), %eax
    orl $(CR4_PSE), %eax
    movl %eax, %cr4

    /* Enter paged mode. */
    movl $kernelPageDir, %eax
    movl %eax, %cr3
    movl %cr0, %eax
    orl  $(CR0_PG), %eax
    movl %eax, %cr0
    
    /* Invoke kernel. */
    call kernel_main
                
/**
 * Stop execution immediately.
 */
halt:
    cli
    hlt
    jmp halt

/**
 * Generated interrupt handlers.
 */
interruptHandler  0, 0
interruptHandler  1, 0
interruptHandler  2, 0
interruptHandler  3, 0
interruptHandler  4, 0
interruptHandler  5, 0
interruptHandler  6, 0
interruptHandler  7, 0
interruptHandler  8, 1
interruptHandler  9, 0
interruptHandler 10, 1
interruptHandler 11, 1
interruptHandler 12, 1
interruptHandler 13, 1
interruptHandler 14, 1
interruptHandler 15, 0
interruptHandler 16, 0
interruptHandler 32, 0
interruptHandler 33, 0
interruptHandler 34, 0
interruptHandler 35, 0
interruptHandler 36, 0
interruptHandler 37, 0
interruptHandler 38, 0
interruptHandler 39, 0
interruptHandler 40, 0
interruptHandler 41, 0
interruptHandler 42, 0
interruptHandler 43, 0
interruptHandler 44, 0
interruptHandler 45, 0
interruptHandler 46, 0
interruptHandler 47, 0
interruptHandler 48, 0
interruptHandler 0x90, 0

.section ".bss"

/**
 * Kernel boot stack.
 */
.align PAGESIZE
stack:  .fill STACK_SIZE, 1, 0

/**
 * Kernel Page Tables.
 */
.align PAGESIZE
kernelPageDir:  .fill PAGESIZE, 1, 0
kernelTss:      .fill PAGESIZE, 1, 0

.section ".data"

/**
 * Global Descriptor Table.
 */
gdt:
        .quad   0x0000000000000000 /* NULL descriptor. */
        .quad   0x00cf9a000000ffff /* Kernel CS. */
        .quad   0x00cf92000000ffff /* Kernel DS. */
        .quad   0x00cffa000000ffff /* User CS. */
        .quad   0x00cff2000000ffff /* User DS. */
        .quad   0x0000000000000000 /* User TSS descriptor. */
gdt_end:

gdtPtr:
    .word gdt_end - gdt
    .long gdt
    .word 0

/**
 * Interrupt Descriptor Table.
 */
idt:
        .fill 256, 8, 0     /* Empty IDT space. */

idtPtr:                     /* 256 IDT entries. */
        .word 256*8-1
        .long idt

.align PAGESIZE
