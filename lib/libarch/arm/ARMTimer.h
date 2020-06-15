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

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libarch
 * @{
 *
 * @addtogroup libarch_arm
 * @{
 */

/**
 * ARM Generic Timer.
 */
class ARMTimer : public Timer
{
  private:

    /** Set this bit in the PL1 control register to enable it. */
    static const u32 TimerControlEnable = 1;

  public:

    /**
     * Constructor
     */
    ARMTimer();

    /**
     * Set timer frequency.
     *
     * @param hertz Frequency of the timer in hertz.
     *
     * @return Result code
     */
    virtual Result setFrequency(const Size hertz);

    /**
     * Process timer tick.
     *
     * Should be called on each timer interrupt to restart the timer
     * by applying the initial timer counter value.
     *
     * @return Result code
     */
    virtual Result tick();

  private:

    /**
     * Retrieve system timer frequency
     *
     * @return System frequency
     */
    u32 getSystemFrequency(void) const;

    /**
     * Set Physical Timer 1 value
     *
     * @param value New timer value
     */
    void setPL1PhysicalTimerValue(const u32 value);

    /**
     * Set Physical Timer 1 control value
     *
     * @param value New timer control value
     */
    void setPL1PhysicalTimerControl(const u32 value);

  private:

    /** Currently configured initial timer counter. */
    Size m_initialTimerCounter;
};

/**
 * @}
 * @}
 * @}
 */

#endif /* __LIBARCH_ARM_ARMTIMER_H */
