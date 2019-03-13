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

#ifndef __LIBARCH_ARM_TIMER_H
#define __LIBARCH_ARM_TIMER_H

#include <Types.h>
#include <Macros.h>
#include <Timer.h>

/** PhysicalTimer1, IRQ number. */
#define GTIMER_PHYS_1_IRQ 3

/**
 * ARM Generic Timer.
 */
class ARMTimer : public Timer
{
  public:

    ARMTimer();

    /**
     * Set timer frequency.
     *
     * @param hertz Frequency of the timer in hertz.
     * @return Result code.
     */
    virtual Result setFrequency(Size hertz);

    /**
     * Process timer tick.
     *
     * Should be called on each Timer interrupt to
     * keep the m_info variable synchronized with the actual hardware.
     * Also clears the timer interrupt flag.
     */
    virtual Result tick();

  private:

    u32 getSystemFrequency(void);
    void setPL1TimerValue(u32 value);
    void setPL1Control(u32 value);
    u64 getPL1TimerCompare(void);
    void setPL1TimerCompare(u64 value);

    Size m_frequency;
};

#endif /* __LIBARCH_ARM_ARMTIMER_H */
