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

#ifndef __X86_PROCESS_H
#define __X86_PROCESS_H

/**   
 * @defgroup x86kernel kernel (x86)  
 * @{   
 */

#ifndef __ASSEMBLY__
#ifdef  __cplusplus

/**
 * Used by kernel/ classes to point back to us.
 */
#define ArchProcess X86Process
class ArchProcess;

#include <kernel/Process.h>
#include "CPU.h"
#include "Memory.h"
#include <Types.h>

/**
 * Process which may execute on an Intel x86 CPU.
 */
class X86Process : public Process
{
    public:

        /**
         * Constructor function.
	 * @param entry Initial EIP register value.
         */
	X86Process(Address entry);

	/**
	 * Destructor function.
	 */
	~X86Process();
	
	/**
	 * (Dis)allows a process direct I/O to a port.
	 * @param port I/O port.
	 * @param enabled (Dis)allow access.
	 */
	void IOPort(u16 port, bool enabled);
	
	/**
	 * Save and restore registers, then perform a context switch.
	 */
	void execute();

	/**
	 * Get the address of our page directory.
	 * @return Page directory address.
	 */
	Address getPageDirectory()
	{
	    return pageDirAddr;
	}
	
	/**
	 * Get the address of our stack.
	 * @return Stack address.
	 */
	Address getStack()
	{
	    return stackAddr;
	}
	
	/**
	 * Sets the address of our stack.
	 * @param addr New stack address.
	 */
	void setStack(Address addr)
	{
	    stackAddr = addr;
	}

    private:
	
	/** Page Directory physical address. */
	Address pageDirAddr;
	
	/** Current stack pointer. */
	Address stackAddr;
	
	/** Kernel stack pointer. */
	Address kernelStackAddr;
	
	/** I/O bitmap physical address. */
	Address ioMapAddr;
};

/**
 * Assembly routine which performs context switches.
 * @param oldStackPtr Pointer to the stackAddr of the currently running X86Process, if any.
 * @param pageDirAddr Address of the page directory.
 * @param stackAddr Address of the user stack.
 * @param kernelTss Pointer to the kernel's TSS.
 * @param kernelStackAddr Address of the kernel stack.
 * @see TSS
 * @see contextSwitch.S
 */
extern C void contextSwitch(Address *oldStack,  Address pageDirAddr,
			    Address  stackAddr, TSS *kernelTss,
			    Address  kernelStackAddr);

#endif /* __cplusplus */
#endif /* __ASSEMBLY__ */

/**
 * @}
 */

#endif /* __X86_PROCESS_H */
