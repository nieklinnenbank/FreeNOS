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

#ifndef __ARM_CORE_H
#define __ARM_CORE_H

#include <Types.h>
#include <Macros.h>
#include <Core.h>

/**
 * Retrieve the IRQ number from CPUState.
 * @return IRQ number.
 */
#define IRQ_REG(state) \
    ((state)->vector - 0x20)

/**
 * We remap IRQ's to interrupt vectors 32-47.
 */
#define IRQ(vector) \
    (vector) + 32

/**
 * Reads the CPU's timestamp counter.
 * @return 64-bit integer.
 */
#define timestamp() 0

/**
 * Reboot the system (by sending the a reset signal on the keyboard I/O port)
 */
#define reboot()

/**
 * Shutdown the machine via ACPI.
 * @note We do not have ACPI yet. Shutdown now has a bit naive implementation.
 * @see http://forum.osdev.org/viewtopic.php?t=16990
 */
#define shutdown()

/**  
 * Puts the CPU in a lower power consuming state. 
 */
#define idle()

/**
 * Explicitely enable interrupts.
 */
#define irq_enable()

/**
 * Disable interrupts, and store current interrupt state.
 * @warning This is dangerous: no guarantee of the current stack state.
 */
#define irq_disable()

/**
 * Restore the previous interrupt state.
 * @warning This is dangerous: no guarantee of the current stack state.
 */
#define irq_restore(saved)

#define tlb_flush_all()

#define tlb_flush(page)

/** 
 * Contains all the CPU registers. 
 */
typedef struct CPUState
{
    u32 vector;
}
CPUState;

#endif /* __ARM_CORE_H */
