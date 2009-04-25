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

#ifndef __KERNEL_KERNEL_H
#define __KERNEL_KERNEL_H

#include <Macros.h>
#include <Types.h>
#include <arch/Interrupt.h>
#include "Multiboot.h"

/** 
 * @defgroup kernel kernel (generic)
 * @{ 
 */

/**
 * Represents the kernel core.
 */
class Kernel
{
    public:
    
	/**
	 * Constructor function.
	 */
	Kernel();

        /**
         * Hooks a function to an hardware interrupt.
         * @param vec Interrupt vector to hook on.
	 * @param h Handler function.
	 * @param p Parameter to pass to the handler function.
	 */
	virtual void hookInterrupt(int vec, InterruptHandler h, ulong p) = 0;

        /** 
         * Enable or disable an hardware interrupt (IRQ). 
         * @param vector IRQ number. 
         * @param enabled True to enable, and false to disable. 
         */
        virtual void enableIRQ(uint vector, bool enabled) = 0;
};

/**
 * Kernel entry point.
 */
extern C void kmain();

/** Start of kernel text and data. */
extern Address kernelStart;

/** End of kernel. */
extern Address kernelEnd;

/**
 * @}
 */

#endif /* __KERNEL_KERNEL_H */
