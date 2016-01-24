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

#ifndef __ARM_KERNEL_H
#define __ARM_KERNEL_H

#include <FreeNOS/Kernel.h>
#include <FreeNOS/Process.h>
#include <Types.h>
#include <arm/ARMTags.h>
#include <arm/broadcom/BroadcomTimer.h>

/** Forward declaration */
class ARMInterrupt;

/** 
 * Perform a kernel trap with 1 argument.
 * @param num Unique number of the handler to execute. 
 * @return An integer. 
 */
inline ulong trapKernel1(ulong api, ulong arg1)
{
    ulong ret;
    asm volatile ("swi #0\n"
                  "mov %[ret], r0\n"
                : [ret]"=r"(ret));
    return ret;
}

/**
 * Perform a kernel trap with 2 arguments.
 */
inline ulong trapKernel2(ulong api, ulong arg1, ulong arg2)
{
    ulong ret;
    asm volatile ("swi #0\n"
                  "mov %[ret], r0\n"
                : [ret]"=r"(ret));
    return ret;
}

/** 
 * Perform a kernel trap with 3 arguments. 
 * @param num Unique number of the handler to execute. 
 * @return An integer. 
 */
inline ulong trapKernel3(ulong api, ulong arg1, ulong arg2, ulong arg3)
{
    ulong ret;
    asm volatile ("swi #0\n"
                  "mov %[ret], r0\n"
                : [ret]"=r"(ret));
    return ret;
}

/** 
 * Perform a kernel trap with 4 arguments. 
 * @param num Unique number of the handler to execute. 
 * @return An integer.
 */
inline ulong trapKernel4(ulong api, ulong arg1, ulong arg2, ulong arg3,
                         ulong arg4)
{
    ulong ret;
    asm volatile ("ldr r4, %[arg4]\n"
                  "swi #0\n"
                  "mov %[ret], r0\n"
                : [ret]"=r"(ret)
                : [arg4]"m"(arg4));
    return ret;
}

/** 
 * Perform a kernel trap with 5 arguments. 
 * @param num Unique number of the handler to execute. 
 * @return An integer. 
 */
inline ulong trapKernel5(ulong api, ulong arg1, ulong arg2, ulong arg3,
                         ulong arg4, ulong arg5)
{
    ulong ret;
    asm volatile ("ldr r4, %[arg4]\n"
                  "ldr r5, %[arg5]\n"
                  "swi #0\n"
                  "mov %[ret], r0\n"
                : [ret]"=r"(ret)
                : [arg4]"m"(arg4),
                  [arg5]"m"(arg5));
    return ret;
}

/**
 * Represents the ARM kernel implementation
 */
class ARMKernel : public Kernel
{
  public:
    
    /**
     * Constructor function.
     *
     * @param kernel Describes the start and end of the kernel program in memory.
     * @param memory Describes the start and end of physical RAM in the system.
     * @param intr ARM interrupt controller implementation.
     * @param atags Address of the ARM boot tags
     */
    ARMKernel(ARMInterrupt *intr,
              CoreInfo *info);

  private:

    static void interrupt(CPUState state);

    static void trap(CPUState state);

    static void undefinedInstruction(CPUState state);

    static void prefetchAbort(CPUState state);

    static void dataAbort(CPUState state);

    static void reserved(CPUState state);

    BroadcomTimer m_bcmTimer;
};

#endif /* __ARM_KERNEL_H */
