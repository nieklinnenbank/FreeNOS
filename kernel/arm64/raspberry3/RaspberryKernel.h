/*
 * Copyright (C) 2019 Niek Linnenbank
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

#ifndef __ARM_KERNEL_RASPBERRYKERNEL_H
#define __ARM_KERNEL_RASPBERRYKERNEL_H

#include <FreeNOS/arm64/ARM64Kernel.h>
#include <arm64/broadcom/BroadcomInterrupt.h>
#include <arm64/broadcom/Broadcom2836.h>
#include <arm64/ARM64Timer.h>
#if 0
#include <arm/broadcom/BroadcomTimer.h>
#endif
/**
 * @addtogroup kernel
 * @{
 *
 * @addtogroup kernel_arm
 * @{
 */

/**
 * Represents the Raspberry Pi kernel implementation
 */
class RaspberryKernel : public ARM64Kernel
{
  public:

    /**
     * Constructor function.
     *
     * @param info Contains processor core specific information
     */
    RaspberryKernel(CoreInfo *info);

  private:

    /**
     * Interrupt handler routine
     *
     * @param state Saved CPU register state
     */
    static void interrupt(CPUState state);

  private:

    /** Broadcom specific interrupt controller */
    BroadcomInterrupt m_bcmIntr;

    /** Broadcom specific registers */
    Broadcom2836 m_bcm;
    /** ARM generic timer. Only used for QEMU */
    ARM64Timer m_armTimer;

#if 0
    /** Broadcom specific timer module */
    BroadcomTimer m_bcmTimer;
#endif
    /** Interrupt number for the timer */
    u8 m_timerIrq;
};

/**
 * @}
 * @}
 */

#endif /* __ARM_KERNEL_RASPBERRYKERNEL_H */
