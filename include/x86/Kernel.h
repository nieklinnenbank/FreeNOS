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

#ifndef __X86_KERNEL_H
#define __X86_KERNEL_H

#ifndef __ASSEMBLY__

#include <kernel/Kernel.h>
#include <Singleton.h>
#include <Types.h>
#include "Interrupt.h"
#include "CPU.h"

/**   
 * @defgroup x86kernel kernel (x86)  
 * @{   
 */

/** IO base address for master PIC */
#define PIC1_CMD        0x20 

/** IO base address for slave PIC */                      
#define PIC2_CMD        0xa0                              
                                                          
/* Master PIC data port */                                
#define PIC1_DATA       0x21                              
                                                          
/* Slave PIC data port */                                 
#define PIC2_DATA       0xa1 

/** End of Interrupt (EOI). */
#define PIC_EOI		0x20

/** Base of IRQ's from the PIC's. */
#define PIC_IRQ_BASE	0x20

/** PIT maximum frequency. */
#define PIT_FREQUENCY   1193182

/** PIT IRQ interval. */
#define PIT_HZ		250

/** PIT divisor. */
#define PIT_DIVISOR	(PIT_FREQUENCY / PIT_HZ)

/** PIT command port. */
#define PIT_CMD		0x43

/** PIT channel zero. */
#define PIT_CHAN0	0x40

/**
 * Implements an x86 compatible kernel.
 */
class X86Kernel : public Kernel, public Singleton<X86Kernel>
{
    public:

        /**
         * Constructor function.
         */
	X86Kernel();
	
	/**
	 * Hooks a function to an hardware interrupt.
	 * @param vec Interrupt vector to hook on.
	 * @param h Handler function.
	 * @param p Parameter to pass to the handler function.
	 */
	void hookInterrupt(int vec, InterruptHandler h, ulong p);

        /** 
         * Uses the PIC to (un)mask an IRQ. 
         * @param vector Interrupt vector. 
         * @param enabled Either to mask (true) or unmask (false). 
         */
        void enableIRQ(uint vector, bool enabled);

    private:
    
	/** 
         * Called when the CPU detects a fault. 
         * @param state Contains CPU registers, interrupt vector and error code. 
         * @param param Not used. 
         */
        static void exception(CPUState *state, ulong param);
	
	/** 
         * Default interrupt handler. 
         * @param state Contains CPU registers, interrupt vector and error code. 
         * @param param Not used. 
         */
        static void interrupt(CPUState *state, ulong param);

	/**
	 * Kernel trap handler (system calls).
	 * @param state Contains the arguments for the APIHandler, in CPU registers.
	 * @param param Not used.
	 */
	static void trap(CPUState *state, ulong param);
	
	/** 
         * i8253 system clock interrupt handler. 
         * @param state CPU registers on time of interrupt. 
	 * @param param Not used.
         */
	static void clocktick(CPUState *state, ulong param);
        
	/** Amount of i8253 ticks (interrupts) counted so far. */
	Size ticks;
};

/** Points to the kernel. */
extern X86Kernel *kernel;

/**
 * @}
 */

#endif /* __ASSEMBLY__ */
#endif /* __X86_CPU_H */
