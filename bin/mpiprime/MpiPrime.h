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

#ifndef __BIN_MPIPRIME_MPIPRIME_H
#define __BIN_MPIPRIME_MPIPRIME_H

#include <POSIXApplication.h>

/**
 * @addtogroup bin
 * @{
 */

/**
 * Calculate prime numbers in parallel.
 */
class MpiPrime : public POSIXApplication
{
  private:

  public:

    /**
     * Constructor
     *
     * @param argc Argument count
     * @param argv Argument values
     */
    MpiPrime(int argc, char **argv);

    /**
     * Destructor
     */
    virtual ~MpiPrime();

    /**
     * Initialize the application.
     *
     * @return Result code
     */
    virtual Result initialize();

    /**
     * Execute the application.
     *
     * @return Result code
     */
    virtual Result exec();

  private:

    Result search_parallel(int k, int n, unsigned *map);

    Result search_sequential(int n, unsigned *map);

    Result collect(int n, unsigned *map);

  private:

    /** Result of MPI initialization */
    int m_mpiInitResult;

    /** MPI core identifier (rank) of the current process */
    int m_id;

    /** Total number of cores */
    int m_cores;
};

/**
 * @}
 */

#endif /* __BIN_MPIPRIME_MPIPRIME_H */
