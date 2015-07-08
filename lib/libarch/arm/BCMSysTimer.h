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

#define __IO(addr) *((u32 *) (addr))

#define SYSTIMER_CS_ADDR  0x20003000
#define SYSTIMER_CLO_ADDR 0x20003004
#define SYSTIMER_C0_ADDR  0x2000300C
#define SYSTIMER_C1_ADDR  0x20003010
#define SYSTIMER_C2_ADDR  0x20003014
#define SYSTIMER_C3_ADDR  0x20003018
#define SYSTIMER_CS  __IO(SYSTIMER_CS_ADDR)
    #define M3 3
    #define M2 2
    #define M1 1
    #define M0 0
#define SYSTIMER_CLO __IO(SYSTIMER_CLO_ADDR)
#define SYSTIMER_C0  __IO(SYSTIMER_C0_ADDR) //the GPU may be using this one, steer clear
#define SYSTIMER_C1  __IO(SYSTIMER_C1_ADDR)
#define SYSTIMER_C2  __IO(SYSTIMER_C2_ADDR) //the GPU may be using this one, steer clear
#define SYSTIMER_C3  __IO(SYSTIMER_C3_ADDR)

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
