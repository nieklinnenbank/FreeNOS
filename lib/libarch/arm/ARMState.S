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

#include "ARMConstant.h"

.global switchCoreState
.section ".text"

/*
 void switchCoreState(Address *currentStack,
                      Address pageDirectory,
                      Address stackAddr);
 */
switchCoreState:

    /* Setup correct stackframe. */

    /* currentStack == 0? */
    cmp r0, #0
    beq restoreState

    /* Save kernel registers on current stack. */
    push {lr}
    push {r0-r12}

    /* Save stack pointer in currentStack */
    stm r0, {sp}

restoreState:

    /* Reload stack */
    mov sp, r2
    mov r4, #0

    /* Reload first page table */
    mcr p15, 0, r4, c7, c5, 0    /* flush entire instruction cache */
    mcr p15, 0, r4, c7, c10, 0   /* flush entire data cache */
    mcr p15, 0, r1, c2, c0, 0    /* switch first page table */

    mcr p15, 0, r4, c8, c5, 0    /* flush instruction TLB */
    mcr p15, 0, r4, c8, c6, 0    /* flush data TLB */
    mcr p15, 0, r4, c8, c7, 0    /* flush unified TLB */

    /* Restore kernel registers. */
    pop {lr}
    pop {r0-r12}
    mov pc, lr
