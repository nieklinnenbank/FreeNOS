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

#ifndef __LIB_LIBARCH_KERNELTIMER_H
#define __LIB_LIBARCH_KERNELTIMER_H

#include "Timer.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libarch
 * @{
 */

/**
 * Provides the timer of the kernel
 *
 * @see ProcessCtl
 */
class KernelTimer : public Timer
{
  public:

    /**
     * Process timer tick.
     *
     * Should be called on each Timer interrupt to
     * keep the m_info variable synchronized with the actual hardware.
     *
     * @return Result code
     */
    virtual Result tick();
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBARCH_KERNELTIMER_H */
