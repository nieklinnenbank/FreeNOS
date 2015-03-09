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

#ifndef __INTEL_PROCESS_H
#define __INTEL_PROCESS_H

/**   
 * @defgroup x86kernel kernel (x86)  
 * @{   
 */

#ifndef __ASSEMBLY__
#if defined(__cplusplus)

#include <FreeNOS/Process.h>
#include "IntelCPU.h"
#include <Types.h>

/**
 * Process which may execute on an Intel x86 CPU.
 */
class IntelProcess : public Process
{
    public:

        /**
         * Constructor function.
	 * @param entry Initial EIP register value.
         */
	IntelProcess(ProcessID id, Address entry);

	/**
	 * Destructor function.
	 */
	virtual ~IntelProcess();
	
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
         * (Dis)allows a process direct I/O to a port.
         * @param port I/O port.
         * @param enabled (Dis)allow access.
         */
        void IOPort(u16 port, bool enabled);

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
 * @param oldStackPtr Pointer to the stackAddr of the currently running IntelProcess, if any.
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

#endif /* __INTEL_PROCESS_H */
