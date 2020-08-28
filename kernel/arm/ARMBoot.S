/*
 * Copyright (C) 2015 Niek Linnenbank
 * Copyright (C) 2013 Goswin von Brederlow <goswin-v-b@web.de>
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

#include <FreeNOS/Constant.h>

#define STACKSIZE   (PAGESIZE * 4)

/* To keep this in the first portion of the binary. */
.section ".text.boot"

/* Make _start global. */
.globl _start, svcStack

/*
 * Entry point for the kernel.
 */
_start:

    /*
     * Check if hypervisor mode is enabled. Some new ARM processors
     * offer hypervisor mode which is very limited in its instruction
     * and execution functionality. The processor must switch to system service mode (SVC)
     * before continuing further with bootstrap.
     */
    mrs r4, cpsr
    and r4, r4, #(MODE_MASK)
    teq r4, #(HYP_MODE)
    bne 1f
    mrs r4, cpsr
    bic r4, r4, #(MODE_MASK)
    orr r4, r4, #(SVC_MODE)
    orr r4, r4, #(IRQ_BIT | FIQ_BIT | ASYNC_ABORT_BIT)
    msr spsr_fsxc, r4
    adr lr, 1f
    ELR
    ERET
1:

#if defined(SMP) && defined(SMP_WAIT)
    /* Read CoreID */
    mrc p15, 0, r0, c0, c0, 5
    and r0, r0, #3
    cmp r0, #0
    beq 3f

2:  /* cpu 1..N */
    wfi
    b 2b
3:  /* cpu 0 */
#endif /* SMP && SMP_WAIT */

    /* Initialize stack pointers for all ARM modes */
    msr cpsr_c, #(IRQ_MODE | IRQ_BIT | FIQ_BIT) /* IRQ mode */
    ldr sp, =(irqStack + STACKSIZE)

    msr cpsr_c, #(FIQ_MODE | IRQ_BIT | FIQ_BIT) /* FIQ mode */
    ldr sp, =(fiqStack + STACKSIZE)

    msr cpsr_c, #(SVC_MODE | IRQ_BIT | FIQ_BIT) /* SVC mode */
    ldr sp, =(svcStack + STACKSIZE)

    msr cpsr_c, #(ABT_MODE | IRQ_BIT | FIQ_BIT) /* Abort mode */
    ldr sp, =(abtStack + STACKSIZE)

    msr cpsr_c, #(UND_MODE | IRQ_BIT | FIQ_BIT) /* Undefined mode */
    ldr sp, =(undStack + STACKSIZE)

    msr cpsr_c, #(SYS_MODE | IRQ_BIT | FIQ_BIT) /* System mode */
    ldr sp, =(sysStack + STACKSIZE)

    /* continue kernel in SVC mode, temporary stack */
    msr cpsr_c, #(SVC_MODE | IRQ_BIT | FIQ_BIT) /* SVC mode */
    ldr sp, =(TMPSTACKADDR + STACKSIZE)

    /* Call kernel_main */
    ldr r3, =kernel_main
    blx r3

.section ".data"
.align 8
.fill STACKSIZE, 1, 0
irqStack:

.fill STACKSIZE, 1, 0
fiqStack:

.fill STACKSIZE, 1, 0
svcStack:

.fill STACKSIZE, 1, 0
abtStack:

.fill STACKSIZE, 1, 0
undStack:

.fill STACKSIZE, 1, 0
sysStack:
