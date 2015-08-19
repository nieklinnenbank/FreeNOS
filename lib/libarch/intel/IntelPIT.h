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

#ifndef __LIBARCH_INTEL_PIT_H
#define __LIBARCH_INTEL_PIT_H

#include <Types.h>
#include <BitOperations.h>

/**
 * Intel 8254 Programmable Interrupt Timer (PIT).
 */
class IntelPIT
{
  private:

    /** Oscillator frequency in hertz used by the PIT. */
    static const uint OscillatorFreq = 1193182;

    /** The IRQ vector for channel 0 */
    static const uint InterruptVec = 0;

    /**
     * Hardware registers.
     */
    enum Registers
    {
        Control      = 0x43,
        Channel0Data = 0x40
    };

    /**
     * Control Register Flags.
     */
    enum ControlFlags
    {
        Channel0      = 0,
        LatchedRead   = 0,
        AccessLowHigh = (3 << 4),
        SquareWave    = (3 << 1)
    };

  public:

    /**
     * Result codes.
     */
    enum Result
    {
        Success,
        NotFound,
        InvalidFrequency
    };

    /**
     * Constructor
     *
     * @param hertz Number of interrupt triggers per second (in hertz)
     */
    IntelPIT(uint hertz = 250);

    /**
     * Get interrupt vector.
     *
     * The interrupt vector for channel 0 is fixed to vector 0.
     *
     * @return Interrupt vector number.
     */
    uint getInterruptVector();

    /**
     * Get current timer counter.
     *
     * The timer counter is used internally by the PIT
     * to generate interrupts on the desired frequency.
     * The PIT will decrement the timer counter until 0
     * is reached, at which it will generate an interrupt
     * and restart at the initial timer counter value.
     *
     * @return Current timer counter.
     */
    uint getCounter();

    /**
     * Get interrupt frequency.
     *
     * @return Current interrupt frequency.
     */
    uint getFrequency();

    /**
     * Set interrupt frequency.
     *
     * This function configures the PIT
     * to generate interrupts on the given frequency.
     * The new frequency will be applied immediately on
     * the interrupts generated.
     *
     * @param hertz Number of interrupt triggers per second (in hertz)
     * @return Result code.
     */
    Result setFrequency(uint hertz);

  private:

    /**
     * Set Control register.
     *
     * @param flags New flags to set in the Control register.
     * @return Result code.
     */
    Result setControl(ControlFlags flags);

    /** Interrupt frequency of the PIT */
    uint m_hertz;
};

#endif /* __LIBARCH_INTEL_PIT_H */
