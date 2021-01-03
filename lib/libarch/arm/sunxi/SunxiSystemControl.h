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

#ifndef __LIBARCH_ARM_SUNXI_SUNXISYSTEMCONTROL_H
#define __LIBARCH_ARM_SUNXI_SUNXISYSTEMCONTROL_H

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
 * Allwinner System Control (SYSCON) module
 */
class SunxiSystemControl
{
  private:

    /** Physical base memory address of the SYSCON module */
    static const Address IOBase = 0x01C00000;

  private:

    /**
     * Hardware registers.
     */
    enum Registers
    {
        EmacClock = 0x30,
    };

    /**
     * EMAC Clock Register flags
     */
    enum EmacClockFlags
    {
        EmacClockDefault  = 0x58000,
        EmacClockMask     = 0xffff8000,
        EmacClockPhyShift = 20,
        EmacClockLedPoll  = (1 << 17),
        EmacClockShutdown = (1 << 16),
        EmacClockSelect   = (1 << 15),
        EmacClockRmiiEn   = (1 << 13),
    };

  public:

    /**
     * Result codes.
     */
    enum Result
    {
        Success,
        IOError,
        InvalidArgument
    };

    /**
     * Perform initialization.
     *
     * @return Result code.
     */
    Result initialize();

    /**
     * Setup EMAC mode
     *
     * @param phyAddr PHY address
     *
     * @return Result code.
     */
    Result setupEmac(const uint phyAddr);

  private:

    /** Memory I/O object */
    Arch::IO m_io;
};

/**
 * @}
 * @}
 * @}
 */

#endif /* __LIBARCH_ARM_SUNXI_SUNXISYSTEMCONTROL_H */
