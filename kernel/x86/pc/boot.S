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

#include <FreeNOS/Multiboot.h>
#include "CPU.h"
#include "Memory.h"

/* The size of our stack (16KB).  */
#define STACK_SIZE 0x4000

/**
 * Generates interrupt handlers.
 */
.macro interruptHandler vnum, verr
i\vnum:
.if !\verr
	pushl $0
.endif
	pushl $\vnum
	jmp invokeHandler
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
        movw %bx, (%eax)        	/* Offset low */
        shrl $16, %ebx
        movw %bx, 6(%eax)       	/* Offset high */
        movw $KERNEL_CS_SEL, 2(%eax)    /* Kernel CS */
        movb $0, 4(%eax)        	/* Zeroes */
        movb $\vtype, 5(%eax)   	/* Present, 32 bits, 01110 */
.endm

.global _start, multibootHeader, multibootInfo, gdt, kernelPageDir, kernelPageTab, kernelTss, kernelioBitMap

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
_start:
	/* Disable interrupts. */
	cli

	/* Setup temporary boot stack. */
	movl $(stack + STACK_SIZE), %esp
	movl %esp, %ebp

	/* Multiboot magic + struct. */
	cmpl $MULTIBOOT_BOOTLOADER_MAGIC, %eax
	jnz halt
	movl $multibootInfo, %eax
	movl $MULTIBOOT_HEADER_SIZE, %ecx
1:
	movl (%ebx), %edx
	movl %edx, (%eax)
	addl $4, %ebx
	addl $4, %eax
	subl $4, %ecx
	jnz 1b

        /* Load GDT. */
	lgdt gdtPtr

	/* Fill in IDT entries 0 - 16, and 32 - 47. */
	idtEntry 0, 0x8f
	idtEntry 1, 0x8f
	idtEntry 2, 0x8f
	idtEntry 3, 0x8f
	idtEntry 4, 0x8f
	idtEntry 5, 0x8f
	idtEntry 6, 0x8f
	idtEntry 7, 0x8f
	idtEntry 8, 0x8f
	idtEntry 9, 0x8f
	idtEntry 10, 0x8f
	idtEntry 11, 0x8f
	idtEntry 12, 0x8f
	idtEntry 13, 0x8f
	idtEntry 14, 0x8f
	idtEntry 15, 0x8f
	idtEntry 16, 0x8f
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
        idtEntry 0x90, 0xee

	/* Load IDT. */
	lidt idtPtr

	/* Setup segments. */
	movl $KERNEL_DS_SEL, %eax
	movl %eax, %ds
	movl %eax, %es
	movl %eax, %ss

	/* Kernel page directory. */
	movl $kernelPageDir, %eax
	movl $kernelPageTab, %ebx
	orl  $(PAGE_PRESENT | PAGE_RW | PAGE_PINNED), %ebx
	movl %ebx, (%eax)

	/* Identity map the first 4MB. */
	movl $kernelPageTab, %eax
	xorl %ebx, %ebx
	orl $(PAGE_PRESENT | PAGE_RW | PAGE_PINNED), %ebx
	movl $1024, %ecx
1:
	movl %ebx, (%eax)
	addl $PAGESIZE, %ebx
	addl $4, %eax
	decl %ecx
	jnz 1b

	/* Map page directory into itself. */
	movl $4, %eax
	movl $PAGETABFROM, %ebx
	shrl $DIRSHIFT, %ebx
	imul %ebx
	movl $kernelPageDir, %ebx
	addl %eax, %ebx
	movl %ebx, %eax
	movl $kernelPageDir, %ebx
	orl  $(PAGE_PRESENT | PAGE_RW | PAGE_PINNED), %ebx
	movl %ebx, (%eax)

	/* Enter paged mode. */
	movl $kernelPageDir, %eax
	movl %eax, %cr3
	movl %cr0, %eax
	orl  $(CR0_PG), %eax
	movl %eax, %cr0
	
	/* Enable timestamp counter. */
	movl %cr4, %eax
	andl  $(~CR4_TSD), %eax
	movl %eax, %cr4

	/* Invoke kernel. */
	call kmain
				
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
interruptHandler 0x90, 0

/**
 * Invokes the correct interrupt handler.
 */
invokeHandler:
        
	/* Make a CPUState. */
	pusha
        pushl %ss
        pushl %ds
        pushl %es
        pushl %fs
        pushl %gs
	
	/* Replace data segments. */
	pushl %eax
	mov $KERNEL_DS_SEL, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	popl %eax
	
	/* Now call the correct C++ handler. */
        call executeInterrupt
	
	/* Restore data segments. */
	popl %gs
	popl %fs
	popl %es
	popl %ds
        add $4, %esp
	
	/* Continue program. */
        popa
        add $8, %esp
        iret

.section ".bss"

/**
 * Multiboot information struct.
 */
multibootInfo: .fill MULTIBOOT_HEADER_SIZE, 1, 0

/**
 * Kernel boot stack.
 */
.align PAGESIZE
stack:	.fill STACK_SIZE, 1, 0

/**
 * Kernel Page Tables.
 */
.align PAGESIZE
kernelPageDir:	.fill PAGESIZE, 1, 0
kernelPageTab:	.fill PAGESIZE, 1, 0
kernelTss:	.fill PAGESIZE, 1, 0
kernelioBitMap:	.fill PAGESIZE, 1, 1

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
        .quad   0x0000000000000000 /* TSS descriptor. */
gdt_end:

gdtPtr:
	.word gdt_end - gdt
	.long gdt
	.word 0

/**
 * Interrupt Descriptor Table.
 */
idt:
        .fill 256, 8, 0		/* Empty IDT space. */

idtPtr:				/* 256 IDT entries. */
        .word 256*8-1
        .long idt

.align PAGESIZE
