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
 
.globl vecTable, handlerTable

/*
 * ARM exception vector table.
 *
 * When an interrupt occurs, the ARM will jump to a predefined
 * memory location, which points effectively an entry in the table below.
 * The table below should be loaded at 0x0. Each entry can only be 4 bytes
 * in size. The LDR (Load Register) instruction will load PC with the corresponding
 * address of the interrupt service routine from the interrupt routine table,
 * which is directly after the jump table in memory.
 *
 */
vecTable:

ldr pc, [pc, #0x18] /* Reset */
ldr pc, [pc, #0x18] /* Undefined Instruction */
ldr pc, [pc, #0x18] /* Software Interrupt */
ldr pc, [pc, #0x18] /* Prefetch Abort */
ldr pc, [pc, #0x18] /* Data Abort */
ldr pc, [pc, #0x18] /* Reserved */
ldr pc, [pc, #0x18] /* IRQ vector */
ldr pc, [pc, #0x18] /* FIQ vector */ 

.long(ex_reset)
.long(ex_undefined)
.long(ex_software)
.long(ex_prefetch)
.long(ex_data)
.long(ex_reserved)
.long(ex_irq)
.long(ex_fiq)

ex_reset:
    push {r0}
    mov r0, #0
    b exceptionHandler

ex_undefined:
    push {r0}
    mov r0, #1
    b exceptionHandler

ex_software:
    push {r0}
    mov r0, #2
    b exceptionHandler

ex_prefetch:
    push {r0}
    mov r0, #3
    b exceptionHandler

ex_data:
    push {r0}
    mov r0, #4
    b exceptionHandler

ex_reserved:
    push {r0}
    mov r0, #5
    b exceptionHandler

ex_irq:
    push {r0}
    mov r0, #6
    b exceptionHandler

ex_fiq:
    push {r0}
    mov r0, #7
    b exceptionHandler

/**
 * General exception handler
 */
exceptionHandler:
    ldr r1, =handlerTable
    mov r2, #4
    mla r1, r0, r2, r1
    ldr r0, [r1]
    mov lr, pc
    bx r0
    pop {r0}
    movs pc, lr

/*
 * ARM interrupt handler table.
 *
 * Contains the addresses of C/C++ interrupt service routines.
 */
handlerTable:
.fill 8, 4, 0
