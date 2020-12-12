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

#include <Log.h>
#include <String.h>
#include <SystemClock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <math.h>
#include <unistd.h>
#include "MpiPrime.h"

MpiPrime::MpiPrime(int argc, char **argv)
    : SievePrime(argc, argv)
    , m_mpiInitResult(MPI_Init(&m_argc, &m_argv))
    , m_id(0)
{
    parser().setDescription("Calculate prime numbers in parallel");
}

MpiPrime::~MpiPrime()
{
    DEBUG("");
}

MpiPrime::Result MpiPrime::initialize()
{
    if (m_mpiInitResult != MPI_SUCCESS)
    {
        ERROR("failed to initialize MPI: result = " << m_mpiInitResult);
        return IOError;
    }

    int result = MPI_Comm_rank(MPI_COMM_WORLD, &m_id);
    if (result != MPI_SUCCESS)
    {
        ERROR("failed to lookup MPI rank: result = " << result);
        return IOError;
    }

    result = MPI_Comm_size(MPI_COMM_WORLD, (int *) &m_cores);
    if (result != MPI_SUCCESS)
    {
        ERROR("failed to lookup total number of cores: result = " << result);
        return IOError;
    }

    return Success;
}

MpiPrime::Result MpiPrime::exec()
{
    int n, k, i;
    u8 *map;
    String output;
    SystemClock t1, t2;

    // Retrieve maximum number (n)
    t1.now();
    n = atoi(arguments().get("NUMBER"));

    // Make sure n is divisible by the number of workers
    if ((n % m_cores) != 0)
    {
        n += m_cores;
        n -= (n % m_cores);
    }

    int n_root = sqrt(n);
    m_numbersPerCore = (n - n_root) / m_cores;
    m_numberStart = n_root + (m_id * m_numbersPerCore);
    m_numberEnd = m_numberStart + m_numbersPerCore;

    // Try to allocate memory
    if ((map = (u8 *) malloc(n * sizeof(u8))) == NULL)
    {
        ERROR("malloc failed: " << strerror(errno));
        return IOError;
    }

    // Initialize map. Clear all entries
    for (i = 0; i < n; i++)
        map[i] = 1;

    // We start with 2
    k = 2;
    t2.now();

    if (m_id == 0)
    {
        printf("Setup: ");
        t1.printDiff(t2);
    }

    // Search for primes until done
    t1.now();
    searchParallel(k, n, map);
    t2.now();

    printf("Search_parallel: ");
    t1.printDiff(t2);
    t1.now();

    // Only the master reports the results.
    if (m_id == 0)
    {
        reportResult(n, map);
    }

    // Free resources
    MPI_Finalize();
    free(map);

    if (m_id == 0)
    {
        t2.now();
        printf("Finalize: ");
        t1.printDiff(t2);
    }

    return Success;
}

MpiPrime::Result MpiPrime::searchParallel(int k, int n, u8 *map)
{
    SystemClock t1, t2;
    int sqrt_of_n = sqrt(n);

    // Find all primes below sqrt(n) sequentially
    t1.now();
    searchSequential(sqrt_of_n, map);

    if (m_id == 0)
    {
        t2.now();
        printf("sequential: ");
        t1.printDiff(t2);

        t1.now();
    }

    // Every worker calculates all primes k .. sqrt(n) sequentially
    // and uses the result to mark it's part of the map, concurrently
    // Note that no communication is needed
    while (k < sqrt_of_n)
    {
        // Prime number?
        if (!map[k])
        {
            k++;
            continue;
        }

        // Mark multiples of k in my range
        for (Size i = m_numberStart; i < m_numberEnd; i++)
        {
            // Do we need to unmark this number? (no prime)
            if (!(i % k))
            {
                map[i] = 0;
            }
        }

        // Look for the next prime
        k++;
    }

    if (m_id == 0)
    {
        t2.now();
        printf("parallel: ");
        t1.printDiff(t2);
    }

    // Collect results of all workers
    t1.now();
    collect(n, map);

    if (m_id == 0)
    {
        t2.now();
        printf("collect: ");
        t1.printDiff(t2);

        t1.now();
    }

    return Success;
}

MpiPrime::Result MpiPrime::collect(int n, u8 *map)
{
    Size copyIndex = m_numbersPerCore + sqrt(n);
    MPI_Status status;

    // Allocate temporary buffer
    u8 *mybuf = (u8 *) malloc(sizeof(u8) * m_numbersPerCore);
    if (mybuf == NULL)
    {
        ERROR("malloc failed: " << strerror(errno));
    }

    // Every worker sends it's results to the master
    if (m_id != 0)
    {
        // Send mybuf to the master
        MPI_Send(&map[m_numberStart], m_numbersPerCore, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
    }
    // The master gathers the parts of the list from every worker
    else
    {
        for (Size i = 1; i < m_cores; i++)
        {
            // Receive from worker
            MPI_Recv(mybuf, m_numbersPerCore, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, &status);

            // Copy inside our buffer
            for (Size j = 0; j < m_numbersPerCore; j++)
            {
                map[copyIndex++] = mybuf[j];
            }
        }
    }

    // Cleanup
    free(mybuf);
    return Success;
}
