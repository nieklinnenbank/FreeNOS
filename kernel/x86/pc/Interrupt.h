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

#ifndef __X86_INTERRUPT_H
#define __X86_INTERRUPT_H

#include "CPU.h"
#include <Types.h>
#include <Macros.h>

/**   
 * @defgroup x86kernel kernel (x86)  
 * @{   
 */

/**
 * Enables interrupts.
 */
#define sti() \
    asm volatile ("sti")

/**
 * Disables interrupts.
 */
#define cli() \
    asm volatile ("cli");

/**
 * Explicitely enable interrupts.
 */
#define irq_enable() \
    sti()

/**
 * Disable interrupts, and store current interrupt state.
 * @warning This is dangerous: no guarantee of the current stack state.
 */
#define irq_disable()				\
({						\
    ulong ret;					\
						\
    asm volatile ("pushfl\n"			\
		  "popl %0" : "=g" (ret) :);	\
    cli();					\
    ret;					\
})

/**
 * Restore the previous interrupt state.
 * @warning This is dangerous: no guarantee of the current stack state.
 */
#define irq_restore(saved)			\
    asm volatile ("push %0\n"			\
		  "popfl\n" :: "g" (saved))

/**
 * We remap IRQ's to interrupt vectors 32-47.
 */
#define IRQ(vector) \
    (vector) + 32

/** 
 * Function which is called when the CPU is interrupted. 
 * @param state State of the CPU on the moment the interrupt occurred. 
 * @param param Optional parameter for the handler. 
 */
typedef void InterruptHandler(CPUState *state, ulong param);

/**
 * Interrupt hook class.
 */
typedef struct InterruptHook
{
    /**
     * Constructor function.
     * @param h Handler function for the hook.
     * @param p Parameter to pass.
     */
    InterruptHook(InterruptHandler *h, ulong p) : handler(h), param(p)
    {
    }
    
    /**
     * Comparision operator.
     * @param i InterruptHook pointer.
     * @return True if equal, false otherwise.
     */
    bool operator == (InterruptHook *i)
    {
	return handler == i->handler && param == i->param;
    }

    /** Executed at time of interrupt. */
    InterruptHandler *handler;
	
    /** Passed to the handler. */
    ulong param;
}
InterruptHook;

/**
 * Called by assembler routine invokeHandler() in boot.S.
 * @param state CPU registers pushed on the stack.
 */
extern C void executeInterrupt(CPUState state);

/**
 * @}
 */

#endif /* __X86_INTERRUPT_H */
