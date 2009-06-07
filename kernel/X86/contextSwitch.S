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

.global contextSwitch
.section ".text"

/*
 * contextSwitch(oldStackPtr, pageDirAddr, stackAddr,
 *               kernelTssAddr, kernelStackAddr)
 */
contextSwitch:

    /* Setup correct stackframe. */
    push %ebp
    mov  %esp, %ebp

    /* Saves the current process state, if any. */
    mov 8(%ebp), %eax
    cmpl $0, %eax
    je restoreState
	
    /* Setup stack for IRETD later on. */
    pushf
    push %cs
    push $resume

    /* Save registers on the current stack (CPUState). */
    pushl $0
    pushl $0
    pusha
    pushl %ss
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs

    /* Save stack pointer of current process in oldStackPtr. */
    pushl %esp
    popl %ebx
    movl %ebx, (%eax)

    /*
     * Switch context to new process.
     */
restoreState:

    /* Retrieve arguments. */
    movl 12(%ebp), %eax
    movl 16(%ebp), %ebx
    movl 20(%ebp), %ecx
    movl 24(%ebp), %edx

    /* Reload page directory, stack and TSS. */    
    movl %eax,     %cr3
    movl %edx,   4(%ecx)
    movl $0x10,  8(%ecx)
    movl %ebx,     %esp

    /* Restore CPU registers. */
    popl %gs
    popl %fs
    popl %es
    popl %ds
    popl %ss
    popa
    addl $8, %esp

    /* Resume execution. */
    iret

resume:
    pop %ebp
    ret
