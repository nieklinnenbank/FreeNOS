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

#ifndef __BIN_PRIME_SIEVEPRIME_H
#define __BIN_PRIME_SIEVEPRIME_H

#include <POSIXApplication.h>

/**
 * @addtogroup bin
 * @{
 */

/**
 * Compute prime numbers using the Sieve of Eratosthenes algorithm
 */
class SievePrime : public POSIXApplication
{
  public:

    /**
     * Constructor
     *
     * @param argc Argument count
     * @param argv Argument values
     */
    SievePrime(int argc, char **argv);

    /**
     * Destructor
     */
    virtual ~SievePrime();

    /**
     * Execute the application.
     *
     * @return Result code
     */
    virtual Result exec();

  protected:

    /**
     * Report the calculated results
     *
     * @param n Maximum number to search for prime numbers
     * @param map Pointer to array with prime numbers
     * @param resultsWritten Number of results written to the output
     * @param offsetNumber Add this constant offset value to each result
     *
     * @return Result code
     */
    Result reportResult(const int n,
                        const u8 *map,
                        Size & resultsWritten,
                        const Size offsetNumber = 0) const;

    /**
     * Perform sequential search for prime numbers
     *
     * @param n Maximum number to search for prime numbers
     * @param map Pointer to array with prime numbers marked
     */
    Result searchSequential(const int n,
                            u8 *map) const;
};

/**
 * @}
 */

#endif /* __BIN_PRIME_SIEVEPRIME_H */
