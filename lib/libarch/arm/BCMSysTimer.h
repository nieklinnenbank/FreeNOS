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

#ifndef __LIBARCH_ARM_BCM_SYSTIMER_H
#define __LIBARCH_ARM_BCM_SYSTIMER_H

#include <Types.h>
#include <Macros.h>

#define SYSTIMER_CS  0x20003000
#define SYSTIMER_CLO 0x20003004

/** Used by GPU */
#define SYSTIMER_C0  0x2000300C
#define SYSTIMER_C1  0x20003010

/** Used by GPU */
#define SYSTIMER_C2  0x20003014
#define SYSTIMER_C3  0x20003018
    #define M3 3
    #define M2 2
    #define M1 1
    #define M0 0

/** BCM Sys Timer runs at 1Mhz */
#define BCM_SYSTIMER_FREQ 1000000

class BCMSysTimer
{
  public:

    void setInterval(u32 hertz);

    void next();

  private:

    u32 m_cycles;
};

#endif /* __LIBARCH_ARM_BCM_SYSTIMER_H */
