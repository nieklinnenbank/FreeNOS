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

#ifndef __LIB_LIBSTD_RANDOMIZER_H
#define __LIB_LIBSTD_RANDOMIZER_H

#include "Types.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libstd
 * @{
 */

/**
 * Produces random integers using the Linear congruential generator algorithm.
 *
 * @see http://en.wikipedia.org/wiki/Linear_congruential_generator
 */
class Randomizer
{
  private:

    /** Used in modulus operation of calculated value */
    static const ulong Modulus = 1 << 31;

    /** Used to multiply calculated value */
    static const ulong Multiplier = 1103515245;

    /** Added to calculated value */
    static const ulong Constant = 12345;

  public:

    /**
     * Set a value as the current state
     *
     * @param value Value to set
     */
    void seed(const ulong value);

    /**
     * Get next randomized value
     *
     * @return Random integer value
     */
    ulong next();

  private:

    /** Current randomizer value */
    static ulong m_value;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBSTD_RANDOMIZER_H */
