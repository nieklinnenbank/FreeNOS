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
#include <arm/broadcom/BroadcomTimer.h>

#ifdef BCM2836
#include <arm/broadcom/Broadcom2836.h>
#include <arm/ARMTimer.h>
#endif /* BCM2836 */

/** Forward declaration */
class ARMInterrupt;

/**
 * @addtogroup kernel
 * @{
 *
 * @addtogroup kernel_arm
 * @{
 */

/**
 * @name ARM Kernel Traps
 *
 * ARM specific software interrupts.
 * These functions are called by the user program to
 * invoke the kernel APIs, also known as system calls.
 *
 * @{
 */

/**
 * Perform a kernel trap with 1 argument.
 *
 * @param api Unique number of the handler to execute.
 *
 * @return An integer.
 */
inline ulong trapKernel1(ulong api, ulong arg1)
{
    ulong ret;
    asm volatile ("swi #0\n"
                  "mov %[ret], r0\n"
                : [ret]"=r"(ret)
                :: "memory");
    return ret;
}

/**
 * Perform a kernel trap with 2 arguments.
 *
 * @param api Unique number of the handler to execute.
 * @param arg1 First argument
 * @param arg2 Second argument
 *
 * @return An integer
 */
inline ulong trapKernel2(ulong api, ulong arg1, ulong arg2)
{
    ulong ret;
    asm volatile ("swi #0\n"
                  "mov %[ret], r0\n"
                : [ret]"=r"(ret)
                :: "memory");
    return ret;
}

/**
 * Perform a kernel trap with 3 arguments.
 *
 * @param api Unique number of the handler to execute.
 * @param arg1 First argument
 * @param arg2 Second argument
 * @param arg3 Third argument
 *
 * @return An integer.
 */
inline ulong trapKernel3(ulong api, ulong arg1, ulong arg2, ulong arg3)
{
    ulong ret;
    asm volatile ("swi #0\n"
                  "mov %[ret], r0\n"
                : [ret]"=r"(ret)
                :: "memory");
    return ret;
}

/**
 * Perform a kernel trap with 4 arguments.
 *
 * @param api Unique number of the handler to execute.
 * @param arg1 First argument
 * @param arg2 Second argument
 * @param arg3 Third argument
 * @param arg4 Fourth argument
 *
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
                : [arg4]"m"(arg4)
                : "memory");
    return ret;
}

/**
 * Perform a kernel trap with 5 arguments.
 *
 * @param api Unique number of the handler to execute.
 * @param arg1 First argument
 * @param arg2 Second argument
 * @param arg3 Third argument
 * @param arg4 Fourth argument
 * @param arg5 Fifth argument
 *
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
                  [arg5]"m"(arg5)
                : "memory");
    return ret;
}

/**
 * @}
 */

/**
 * Represents the ARM kernel implementation
 */
class ARMKernel : public Kernel
{
  public:

    /**
     * Constructor function.
     *
     * @param intr ARM interrupt controller implementation.
     * @param info Contains processor core specific information
     */
    ARMKernel(ARMInterrupt *intr,
              CoreInfo *info);

  private:

    /**
     * Interrupt handler routine
     *
     * @param state Saved CPU register state
     */
    static void interrupt(CPUState state);

    /**
     * Software trap routine
     *
     * @param state Saved CPU register state
     */
    static void trap(CPUState state);

    /**
     * Undefined instruction routine
     *
     * @param state Saved CPU register state
     */
    static void undefinedInstruction(CPUState state);

    /**
     * Prefetch abort routine
     *
     * @param state Saved CPU register state
     */
    static void prefetchAbort(CPUState state);

    /**
     * Data abort routine
     *
     * @param state Saved CPU register state
     */
    static void dataAbort(CPUState state);

    /**
     * Reserved routine
     *
     * @param state Saved CPU register state
     */
    static void reserved(CPUState state);

  private:

#ifdef BCM2836
    /** Broadcom specific registers */
    Broadcom2836 m_bcm;

    /** ARM generic timer. Only used for QEMU */
    ARMTimer m_armTimer;
#endif /* BCM2836 */

    /** Broadcom specific timer module */
    BroadcomTimer m_bcmTimer;

    /** Interrupt number for the timer */
    u8 m_timerIrq;
};

/**
 * @}
 * @}
 */

#endif /* __ARM_KERNEL_H */
