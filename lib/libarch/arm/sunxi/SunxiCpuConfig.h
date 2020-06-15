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

#ifndef __LIBARCH_ARM_SUNXI_SUNXICPUCONFIG_H
#define __LIBARCH_ARM_SUNXI_SUNXICPUCONFIG_H

#include <FreeNOS/System.h>
#include <CoreManager.h>
#include "SunxiPowerManagement.h"

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
 * Allwinner sunxi CPU configuration module support
 */
class SunxiCpuConfig : public CoreManager
{
  private:

    /** Number of CPU processor cores is fixed */
    static const Size NumberOfCores = 4;

    /** Physical base memory address of CPU Configuration Module */
    static const Address IOBase = 0x01F01C00;

    /**
     * Hardware registers.
     */
    enum Registers
    {
        CpusRstCtrl  = 0x0000, /**< CPUs Reset Control */
        Cpu0RstCtrl  = 0x0040, /**< CPU#0 Reset Control */
        Cpu0Ctrl     = 0x0044, /**< CPU#0 Control */
        Cpu0Status   = 0x0048, /**< CPU#0 Status */
        Cpu1RstCtrl  = 0x0080, /**< CPU#1 Reset Control */
        Cpu1Ctrl     = 0x0084, /**< CPU#1 Control */
        Cpu1Status   = 0x0088, /**< CPU#1 Status */
        Cpu2RstCtrl  = 0x00C0, /**< CPU#2 Reset Control */
        Cpu2Ctrl     = 0x00C4, /**< CPU#2 Control */
        Cpu2Status   = 0x00C8, /**< CPU#2 Status */
        Cpu3RstCtrl  = 0x0100, /**< CPU#3 Reset Control */
        Cpu3Ctrl     = 0x0104, /**< CPU#3 Control */
        Cpu3Status   = 0x0108, /**< CPU#3 Status */
        Cpu0PwrClamp = 0x0120, /**< CPU#0 Power Clamp */
        Cpu1PwrClamp = 0x0124, /**< CPU#0 Power Clamp */
        Cpu2PwrClamp = 0x0128, /**< CPU#0 Power Clamp */
        Cpu3PwrClamp = 0x012C, /**< CPU#0 Power Clamp */
        CpuSysRst    = 0x0140, /**< CPU System Reset */
        ClkGating    = 0x0144, /**< CPU Clock Gating */
        GenCtrl      = 0x0184, /**< General Control */
        SuperStandby = 0x01A0, /**< Super Standby Flag */
        EntryAddr    = 0x01A4, /**< Reset Entry Address */
        DbgExtern    = 0x01E4, /**< Debug External */
        Cnt64Ctrl    = 0x0280, /**< 64-bit Counter Control */
        Cnt64Low     = 0x0284, /**< 64-bit Counter Low */
        Cnt64High    = 0x0288, /**< 64-bit Counter High */
    };

    /**
     * CPU#X Reset Control flags
     */
    enum CpuRstCtrlFlags
    {
        CpuCoreReset = (1 << 1)
    };

  public:

    /**
     * Perform initialization.
     *
     * @return Result code.
     */
    virtual Result initialize();

    /**
     * Discover processors.
     *
     * @return Result code.
     */
    virtual Result discover();

    /**
     * Boot a processor.
     *
     * @param info CoreInfo object pointer.
     *
     * @return Result code.
     */
    virtual Result boot(CoreInfo *info);

  private:

    /** Memory I/O object */
    Arch::IO m_io;

    /** Power Management module */
    SunxiPowerManagement m_power;
};

/**
 * @}
 * @}
 * @}
 */

#endif /* __LIBARCH_ARM_SUNXI_SUNXICPUCONFIG_H */
