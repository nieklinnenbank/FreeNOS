/*
 * Copyright (C) 2020 Niek Linnenbank
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

#ifndef __LIBARCH_ARM_SUNXI_SUNXICLOCKCONTROL_H
#define __LIBARCH_ARM_SUNXI_SUNXICLOCKCONTROL_H

#include <FreeNOS/System.h>
#include <Types.h>

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
 * Allwinner Clock Control Unit (CCU)
 */
class SunxiClockControl
{
  private:

    /** Physical base memory address of the CCU module */
    static const Address IOBase = 0x01C20000;

  private:

    /**
     * Hardware registers.
     */
    enum Registers
    {
    };

  public:

    /**
     * Clock identifiers
     */
    enum Clock
    {
        ClockEmacTx = 1,
        ClockEphy,
    };

    /**
     * Reset signal identifiers
     */
    enum Reset
    {
        ResetEmacTx = 1,
        ResetEphy,
    };

    /**
     * Result codes.
     */
    enum Result
    {
        Success,
        IOError,
        InvalidArgument
    };

  public:

    /**
     * Perform initialization.
     *
     * @return Result code.
     */
    Result initialize();

    /**
     * Enable a clock
     *
     * @param clock Clock identification
     *
     * @return Result code.
     */
    Result enable(const Clock clock);

    /**
     * De-assert a reset signal
     *
     * @param reset Reset signal identification
     *
     * @return Result code
     */
    Result deassert(const Reset reset);

  private:

    /** Memory I/O object */
    Arch::IO m_io;
};

/**
 * @}
 * @}
 * @}
 */

#endif /* __LIBARCH_ARM_SUNXI_SUNXICLOCKCONTROL_H */
