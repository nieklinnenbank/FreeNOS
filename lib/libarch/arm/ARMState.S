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

switchCoreState:

    /* Setup correct stackframe. */

    /* currentStack == 0? */
    cmp r0, #0
    beq restoreState

    /* Save kernel registers on current stack. */
    push {r0-r12}
    push {lr}

    /* Save stack pointer in currentStack */
    stm r0, {sp}

restoreState:

    /* Reload stack */
    mov sp, r1
    mov r4, #0

    /* Restore kernel registers. */
    pop {lr}
    pop {r0-r12}
    mov pc, lr
