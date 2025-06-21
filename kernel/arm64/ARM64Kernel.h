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

#ifndef __ARM64_KERNEL_H
#define __ARM64_KERNEL_H

#include <FreeNOS/tiny/Kernel.h>
#include <arm64/ARM64Exception.h>
#include <Types.h>

/**
 * @addtogroup kernel
 * @{
 *
 * @addtogroup kernel_arm
 * @{
 */

/**
 * Represents the ARM64 kernel implementation
 */
class ARM64Kernel : public Kernel
{
  public:

    /**
     * Constructor function.
     *
     * @param info Contains processor core specific information
     */
    ARM64Kernel(CoreInfo *info);

  private:
#if 0
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
#endif
    static void SyncExceptionEL1(struct CPUState state);
    static void FatalHandler(struct CPUState state);
  protected:

    /** ARM64 exception handling subsystem. */
    ARM64Exception m_exception;
};

/**
 * @}
 * @}
 */

#endif /* __ARM64_KERNEL_H */
