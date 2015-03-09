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

#ifndef __KERNEL_H
#define __KERNEL_H

#include <System/Function.h>
#include <Macros.h>
#include <Types.h>
#include <Singleton.h>
#include "BootImage.h"
#include "Process.h"
#include "Memory.h"
#include "ProcessFactory.h"
#include "ProcessManager.h"
#include "API.h"

/** 
 * @defgroup kernel kernel (generic)
 * @{ 
 */

/**
 * Represents the kernel core.
 */
class Kernel : public Singleton<Kernel>
{
  public:

    /**
     * Constructor function.
     *
     * @param memory Pointer to a Memory implementation
     */
    Kernel(Memory *memory, ProcessManager *proc);

    /**
     * Get memory.
     */
    Memory * getMemory();

    /**
     * Get process manager.
     */
    ProcessManager * getProcessManager();

    /**
     * Execute the kernel.
     */
    void run();

    /**
     * Execute a generic API function.
     */
    virtual Error invokeAPI(APINumber number,
                            ulong arg1, ulong arg2, ulong arg3, ulong arg4, ulong arg5);

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

protected:

    /**
     * Loads the boot image.
     */
    bool loadBootImage();
    
    /**
     * Creates a new Process from a BootProcess.
     * @param image BootImage pointer loaded by the bootloader in kernel virtual memory.
     * @param imagePAddr Physical memory address of the boot image.
     * @param index Index in the BootProcess table.
     */
    void loadBootProcess(BootImage *image, Address imagePAddr, Size index);

    /** Memory object */
    Memory *m_memory;

    /** Process Manager */
    ProcessManager *m_procs;

    /** API handlers */
    Array<APIHandler> m_apis;
};

/** Start of kernel text and data. */
extern Address kernelStart;

/** End of kernel. */
extern Address kernelEnd;

/**
 * @}
 */

#endif /* __KERNEL_H */
