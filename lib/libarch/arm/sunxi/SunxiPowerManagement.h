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

#ifndef __LIBARCH_ARM_SUNXI_SUNXIPOWERMANAGEMENT_H
#define __LIBARCH_ARM_SUNXI_SUNXIPOWERMANAGEMENT_H

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
 * Allwinner Power, Reset and Clock Management (PRCM) module.
 */
class SunxiPowerManagement
{
  private:

    /** Physical base memory address of the PRCM module */
    static const Address IOBase = 0x01F01400;

    /** Total number of cores supported */
    static const Size NumberOfCores = 4;

  private:

    /**
     * Hardware registers.
     */
    enum Registers
    {
        CpuPowerOff   = 0x100,
        CpuPowerClamp = 0x140
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
     * Power on a processor.
     *
     * @param coreId Number of the core to power on.
     *
     * @return Result code.
     */
    Result powerOnCore(const Size coreId);

  private:

    /** Memory I/O object */
    Arch::IO m_io;
};

/**
 * @}
 * @}
 * @}
 */

#endif /* __LIBARCH_ARM_SUNXI_SUNXIPOWERMANAGEMENT_H */
