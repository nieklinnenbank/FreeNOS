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
#include "ARMControl.h"

/**
 * Retrieve the IRQ number from CPUState.
 * This does not work for ARM. See BCM2835Interrupt instead.
 * @return IRQ number.
 */
#define IRQ_REG(state) \
    (0)
//(state)->r0)

/**
 * We remap IRQ's to interrupt vectors 32-47.
 */
#define IRQ(vector) (vector)

/**
 * Move to ARM from CoProcessor (MRC).
 *
 * Read data from a CoProcessor connected to the ARM.
 */
#define mrc(coproc, opcode1, opcode2, reg, subReg) \
({ \
    ulong r; \
    asm volatile("mrc " QUOTE(coproc) ", " QUOTE(opcode1) ", %0, " QUOTE(reg) ", " QUOTE(subReg) ", " QUOTE(opcode2) "\n" : "=r"(r) : ); \
    r; \
})

/**
 * Move to CoProcessor from ARM (MCR).
 *
 * Write data to CoProcessor connected to the ARM.
 */
#define mcr(coproc, opcode1, opcode2, reg, subReg, value) \
({ \
    u32 val = (value); \
    asm volatile("mcr " QUOTE(coproc) ", " QUOTE(opcode1) ", %0, " QUOTE(reg) ", " QUOTE(subReg) ", " QUOTE(opcode2) "\n" : : "r"(val)); \
})

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
#define irq_enable() \
({ \
    asm volatile("mrs r0,cpsr;" \
                 "bic r0, r0, #0x180;" /* both IRQ and FIQ */ \
                 "msr cpsr_c,r0"); \
})

/**
 * Disable interrupts, and store current interrupt state.
 * @warning This is dangerous: no guarantee of the current stack state.
 */
#define irq_disable() \
({ \
    asm volatile("mrs r0,cpsr;" \
                 "orr r0, r0, #0x180;" /* both IRQ and FIQ */ \
                 "msr cpsr_c,r0"); \
})

/**
 * Restore the previous interrupt state.
 * @warning This is dangerous: no guarantee of the current stack state.
 */
#define irq_restore(saved)

#define tlb_flush_all() \
({ \
    ARMControl ctrl; \
    ctrl.write(ARMControl::InstructionTLBClear, 0); \
    ctrl.write(ARMControl::DataTLBClear, 0); \
    ctrl.write(ARMControl::UnifiedTLBClear, 0); \
})

#define tlb_flush(page) tlb_flush_all()

/**
 * Data Memory Barrier
 */
inline void dmb()
{
    asm volatile("mcr p15, 0, r12, c7, c10, 5");
}

/** 
 * Contains all the CPU registers. 
 */
typedef struct CPUState
{
    u32 padding[4];
    u32 cpsr;
    u32 sp, lr;
    u32 r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12;
    u32 pc;
}
CPUState;

/**
 *
 */
class ARMCore : public Core
{
  public:

    enum Result
    {
        Success = 0,
    };
};

#endif /* __ARM_CORE_H */
