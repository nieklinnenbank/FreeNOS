/*
 * Copyright (C) 2025 Ivan Tan
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

#include <FreeNOS/Kernel.h>
#include <FreeNOS/Process.h>
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
    /**
     * Software trap routine
     *
     * @param state Saved CPU register state
     */
    static void trap(volatile CPUState &state);

    /**
     * Synchronous exceptions from EL1
     *
     * @param state Saved CPU register state
     */
    static void SyncExceptionEL1(volatile CPUState state);
    
    /**
     * Synchronous exceptions from EL0
     *
     * @param state Saved CPU register state
     */
    static void SyncExceptionEL0(volatile CPUState state);
    
    /**
     * Fatal errors
     *
     * @param state Saved CPU register state
     */
    static void FatalHandler(volatile CPUState state);
  protected:

    /** ARM64 exception handling subsystem. */
    ARM64Exception m_exception;
};

/**
 * @}
 * @}
 */

#endif /* __ARM64_KERNEL_H */
