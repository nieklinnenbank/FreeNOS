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

#ifndef __INTEL_KERNEL_H
#define __INTEL_KERNEL_H

#include <Types.h>
#include <BootImage.h>

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
#define PIC_EOI         0x20

/** Base of IRQ's from the PIC's. */
#define PIC_IRQ_BASE    0x20

/** PIT maximum frequency. */
#define PIT_FREQUENCY   1193182

/** PIT IRQ interval. */
#define PIT_HZ          250

/** PIT divisor. */
#define PIT_DIVISOR     (PIT_FREQUENCY / PIT_HZ)

/** PIT command port. */
#define PIT_CMD         0x43

/** PIT channel zero. */
#define PIT_CHAN0       0x40

/**
 * We remap IRQ's to interrupt vectors 32-47.
 */
#define IRQ(vector) \
    (vector) + 32

/**  
 * @group Intel Kernel Traps
 *
 * These functions are called by the user program to
 * invoke the kernel APIs.
 *
 * @{  
 */

/** 
 * Perform a kernel trap with 1 argument.
 * @param num Unique number of the handler to execute. 
 * @param arg1 First argument becomes ECX. 
 * @return An integer. 
 */
inline ulong trapKernel1(ulong num, ulong arg1)
{
    ulong ret;
    asm volatile ("int $0x90" : "=a"(ret) : "a"(num), "c"(arg1));
    return ret;
}

/** 
 * Perform a kernel trap with 2 arguments.
 * @param num Unique number of the handler to execute. 
 * @param arg1 First argument becomes ECX. 
 * @param arg2 Second argument becomes EBX.
 * @return An integer. 
 */
inline ulong trapKernel2(ulong num, ulong arg1, ulong arg2)
{
    ulong ret;
    asm volatile ("int $0x90" : "=a"(ret) : "a"(num), "c"(arg1), "b"(arg2));
    return ret;
}

/** 
 * Perform a kernel trap with 3 arguments. 
 * @param num Unique number of the handler to execute. 
 * @param arg1 First argument becomes ECX. 
 * @param arg2 Second argument becomes EBX. 
 * @param arg3 Third argument becomes EDX. 
 * @return An integer. 
 */
inline ulong trapKernel3(ulong num, ulong arg1, ulong arg2, ulong arg3)
{
    ulong ret;
    asm volatile ("int $0x90" : "=a"(ret) : "a"(num), "c"(arg1), "b"(arg2),
                        "d"(arg3));
    return ret;
}

/** 
 * Perform a kernel trap with 4 arguments. 
 * @param num Unique number of the handler to execute. 
 * @param arg1 First argument becomes ECX. 
 * @param arg2 Second argument becomes EBX. 
 * @param arg3 Third argument becomes EDX. 
 * @param arg4 Fourth argument becomes ESI.
 * @return An integer. 
 */
inline ulong trapKernel4(ulong num, ulong arg1, ulong arg2, ulong arg3,
             ulong arg4)
{
    ulong ret;
    asm volatile ("int $0x90" : "=a"(ret) : "a"(num), "c"(arg1), "b"(arg2),
                        "d"(arg3), "S"(arg4));
    return ret;
}

/** 
 * Perform a kernel trap with 5 arguments. 
 * @param num Unique number of the handler to execute. 
 * @param arg1 First argument becomes ECX. 
 * @param arg2 Second argument becomes EBX. 
 * @param arg3 Third argument becomes EDX. 
 * @param arg4 Fourth argument becomes ESI.
 * @param arg5 Fifth argument becomes EDI.
 * @return An integer. 
 */
inline ulong trapKernel5(ulong num, ulong arg1, ulong arg2, ulong arg3,
             ulong arg4, ulong arg5)
{
    ulong ret;
    asm volatile ("int $0x90" : "=a"(ret) : "a"(num), "c"(arg1), "b"(arg2),
                 "d"(arg3), "S"(arg4), "D"(arg5));
    return ret;
}

/**
 * @}
 */

#include <FreeNOS/Kernel.h>

/**
 * Implements an x86 compatible kernel.
 */
class IntelKernel : public Kernel
{
  public:

    /**
     * Constructor function.
     */
    IntelKernel(Address kernel, Size size, Size memorySize);

    /** 
     * Uses the PIC to (un)mask an IRQ. 
     * @param vector Interrupt vector. 
     * @param enabled Either to mask (true) or unmask (false). 
     */
    virtual void enableIRQ(uint vector, bool enabled);

    /**
     * Loads the boot image.
     */
    virtual bool loadBootImage();

  private:

    /**
     * Creates a new Process from a BootProcess.
     * @param image BootImage pointer loaded by the bootloader in kernel virtual memory.
     * @param imagePAddr Physical memory address of the boot image.
     * @param index Index in the BootProcess table.
     */
    void loadBootProcess(BootImage *image, Address imagePAddr, Size index);
    
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
};

/**
 * @}
 */

#endif /* __INTEL_KERNEL_H */
