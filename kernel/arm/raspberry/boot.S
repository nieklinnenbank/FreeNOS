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

/* To keep this in the first portion of the binary. */
.section ".text.boot"
 
/* Make _start global. */
.globl _start
 
/* Entry point for the kernel.
 * r15 -> should begin execution at 0x8000.
 * r0 -> 0x00000000
 * r1 -> 0x00000C42
 * r2 -> 0x00000100 - start of ATAGS
 * preserve these registers as argument for kernel_main
 */
_start:
	/* Setup the stack. */
	mov sp, #0x8000
 
	/* Clear out bss. */
	ldr r4, =__bss_start
	ldr r9, =__bss_end
	mov r5, #0
	mov r6, #0
	mov r7, #0
	mov r8, #0
	b       2f
 
1:
	/* store multiple at r4. */
	stmia r4!, {r5-r8}
 
	/* If we are still below bss_end, loop. */
2:
	cmp r4, r9
	blo 1b
 
	/* Call kmain */
	ldr r3, =kmain
	blx r3
