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

#ifndef __AMD64_PROCESS_H
#define __AMD64_PROCESS_H

/**   
 * @defgroup x86kernel kernel (x86)  
 * @{   
 */

#ifndef __ASSEMBLY__
#ifdef  __cplusplus

/**
 * Used by kernel/ classes to point back to us.
 */
#define ArchProcess x86Process
class ArchProcess;

#include <kernel/Process.h>
#include "CPU.h"
#include "Memory.h"
#include <Types.h>

/**
 * Process which may execute on an Intel x86 CPU.
 */
class x86Process : public Process
{
    public:

        /**
         * Constructor function.
	 * @param entry Initial EIP register value.
         */
	x86Process(Address entry);

	/**
	 * Destructor function.
	 */
	~x86Process();
	
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

#endif /* __cplusplus */
#endif /* __ASSEMBLY__ */

/**
 * @}
 */

#endif /* __X86_PROCESS_H */
