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

#include <FreeNOS/Kernel.h>
#include <intel/IntelPIT.h>
#include <intel/IntelPIC.h>
#include <intel/IntelAPIC.h>
#include <Timer.h>
#include <Types.h>
#include <BootImage.h>

/**
 * @addtogroup kernel
 * @{
 *
 * @addtogroup kernel_intel
 * @{
 */

/**
 * Implements an x86 compatible kernel.
 *
 * @todo Debugging the kernel under GDB 9.2 on Ubuntu 20.04 seems to give a malformed/corrupted stacktrace
 *       It only happens with intel/pc on compilers g++-8 and g++-9 and g++-10.
 */
class IntelKernel : public Kernel
{
  public:

    /**
     * Constructor function.
     */
    IntelKernel(CoreInfo *info);

    /**
     * Enable or disable an hardware interrupt (IRQ).
     *
     * @param irq IRQ number.
     * @param enabled True to enable, and false to disable.
     */
    virtual void enableIRQ(u32 irq, bool enabled);

  private:

    /**
     * Called when the CPU detects a fault.
     *
     * @param state Contains CPU registers, interrupt vector and error code.
     * @param param Not used.
     * @param vector Not used.
     */
    static void exception(CPUState *state, ulong param, ulong vector);

    /**
     * Default interrupt handler.
     *
     * @param state Contains CPU registers, interrupt vector and error code.
     * @param param Not used.
     * @param vector Not used.
     */
    static void interrupt(CPUState *state, ulong param, ulong vector);

    /**
     * Kernel trap handler (system calls).
     *
     * @param state Contains the arguments for the APIHandler, in CPU registers.
     * @param param Not used.
     * @param vector Not used.
     */
    static void trap(CPUState *state, ulong param, ulong vector);

    /**
     * i8253 system clock interrupt handler.
     *
     * @param state CPU registers on time of interrupt.
     * @param param Not used.
     * @param vector Not used.
     */
    static void clocktick(CPUState *state, ulong param, ulong vector);

  private:

    /** PIT timer instance */
    IntelPIT m_pit;

    /** APIC instance (used if available) */
    IntelAPIC m_apic;

    /** PIC instance */
    IntelPIC m_pic;
};

/**
 * @}
 * @}
 */

#endif /* __INTEL_KERNEL_H */
