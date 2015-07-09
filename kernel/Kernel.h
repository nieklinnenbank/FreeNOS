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

#include <Macros.h>
#include <Types.h>
#include <Singleton.h>
#include <Memory.h>
#include <BootImage.h>
#include "Process.h"
#include "Memory.h"
#include "ProcessManager.h"

/** Forward declaration. */
class API;
class BitAllocator;
struct CPUState;

/**
 * Function which is called when the CPU is interrupted. 
 *
 * @param state State of the CPU on the moment the interrupt occurred. 
 * @param param Optional parameter for the handler. 
 */
typedef void InterruptHandler(struct CPUState *state, ulong param);
    
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
 * @defgroup kernel kernel (generic)
 * @{ 
 */

/**
 * FreeNOS kernel implementation.
 */
class Kernel : public Singleton<Kernel>
{
  public:

    /**
     * Constructor function.
     *
     * @param kernel Describes the start and end of the kernel program in memory.
     * @param memory Describes the start and end of physical RAM in the system.
     */
    Kernel(Memory::Range kernel, Memory::Range memory);

    /**
     * Initialize heap.
     *
     * This function sets up the kernel heap for
     * dynamic memory allocation with new() and delete()
     * operators. It must be called before any object
     * is created using new().
     *
     * @return Zero on success or error code on failure.
     */
    static Error heap(Address base, Size size);

    /**
     * Get physical memory allocator.
     *
     * @return BitAllocator object pointer
     */
    BitAllocator * getMemory();

    /**
     * Get process manager.
     *
     * @return Kernel ProcessManager object pointer.
     */
    ProcessManager * getProcessManager();

    /**
     * Get API.
     *
     * @return Kernel API object pointer.
     */
    API * getAPI();

    /**
     * BootImage physical address.
     */
    Address getBootImageAddress();

    /**
     * BootImage size
     */
    Size getBootImageSize();

    /**
     * Execute the kernel.
     */
    int run();

    /**
     * Hooks a function to an hardware interrupt.
     *
     * @param vec Interrupt vector to hook on.
     * @param h Handler function.
     * @param p Parameter to pass to the handler function.
     */
    virtual void hookInterrupt(u32 vec, InterruptHandler h, ulong p);

    /**
     * Execute an interrupt handler.
     *
     * @param vec Interrupt Vector.
     * @param state CPU state.
     */
    virtual void executeInterrupt(u32 vec, CPUState *state);

    /** 
     * Enable or disable an hardware interrupt (IRQ). 
     * @param vector IRQ number. 
     * @param enabled True to enable, and false to disable. 
     */
    virtual void enableIRQ(u32 vector, bool enabled) = 0;

    /**
     * Loads the boot image.
     */
    virtual bool loadBootImage() = 0;

    /**
     * Load a boot program.
     *
     * @param image BootImage pointer loaded by the bootloader in kernel virtual memory.
     * @param imagePAddr Physical memory address of the boot image.
     * @param index Index in the BootProcess table.
     */
    virtual void loadBootProcess(BootImage *image, Address imagePAddr, Size index);

  protected:

    /** Physical memory allocator */
    BitAllocator *m_memory;

    /** Process Manager */
    ProcessManager *m_procs;

    /** API handlers object */
    API *m_api;

    /** Physical address of the BootImage */
    Address m_bootImageAddress;

    /** Size of the boot image in bytes */
    Size m_bootImageSize;

    /** Interrupt handlers. */
    Vector<List<InterruptHook *> *> m_interrupts;
};

/**
 * @}
 */

#endif /* __KERNEL_H */
