#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

//#include <sys/time.h>
//#include "measure.h"

// Start and end
#define PERNODE(id, nids, base, n)  (((n) - (base)) / (nids))
#define START(id, nids, base, n)    ((base) + ((id) * PERNODE(id, nids, base, n)))
#define END(id, nids, base, n)      (START(id, nids, base, n) + PERNODE(id, nids, base, n))

static int rank, total, length;
static char name[64];

void collect(int n, unsigned *map)
{
    int i, j, sqrt_of_n = sqrt(n), z;
    MPI_Status status;
    unsigned *mybuf = (unsigned *) malloc(sizeof(unsigned) * PERNODE(rank, total, sqrt_of_n, n));

    // Every worker sends it's results to the master
    if (rank != 0)
    {
        // Send mybuf to the master
        MPI_Send(&map[START(rank, total, sqrt_of_n, n)], PERNODE(rank, total, sqrt_of_n, n), MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    // The master gathers the parts of the list from every worker
    else
    {
        for (i = 1; i < total; i++)
        {
            // Receive from worker
            printf("waiting for core%d\n", i);
            MPI_Recv(mybuf, PERNODE(rank, total, sqrt_of_n, n), MPI_INT, i, 0, MPI_COMM_WORLD, &status);

            // Copy inside our buffer
            for (j = START(i, total, sqrt_of_n, n), z = 0; j < END(i, total, sqrt_of_n, n); j++)
            {
                map[j] = mybuf[z++];
            }
        }
    }
    // Cleanup
    free(mybuf);
}

void search_sequential(int k, int n, unsigned *map, int argc, char **argv)
{
    int i, j;

    // Sequential algorithm
    // Next is a prime
    for (i = 2; i < n; i++)
    {
        // Prime number?
        if (map[i])
        {
            // Mask off all multiples
            for (j = i + 1; j < n; j++)
            {
                if (!(j % i))
                    map[j] = 0;
            }
        }
    }
}

void search_parallel(int k, int n, unsigned *map, int argc, char **argv)
{
    int i, last, sqrt_of_n;


    // Make sure n is divisible by the number of workers
    if ((n % total) != 0)
    {
        n += total;
        n -= (n % total);
    }
    sqrt_of_n = sqrt(n);

    // Find all primes below sqrt(n) sequentially
    search_sequential(k, sqrt_of_n, map, argc, argv);

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
        i    = START(rank, total, sqrt_of_n, n);
        last = END(rank, total, sqrt_of_n, n);

        while (i < last)
        {
            // Do we need to unmark this number? (no prime)
            if (!(i % k))
                map[i] = 0;

            i++;
        }
        // Look for the next prime
        k++;
    }
    // Collect results of all workers
    collect(n, map);
}

int main(int argc, char **argv)
{
    int n, k, i;
    unsigned *map;
    //struct timeval t1, t2;

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &total);
    MPI_Get_processor_name(name, &length);

    // Check arguments
    if (argc < 2)
    {
        printf("usage: %s <number>\n", argv[0]);
        return EXIT_FAILURE;
    }
    n = atoi(argv[1]);

    // Initialize map. Clear all entries
    map = (unsigned *) malloc(sizeof(unsigned) * n);
    for (i = 0; i < n; i++)
        map[i] = 1;

    // We start with 2
    k = 2;

    // begin measuring
    //measure_start(&t1, &t2);

    // Search for primes until done
    //search_sequential(k, n, map, argc, argv);
    search_parallel(k, n, map, argc, argv);

    // end measuring
    //measure_stop(&t1, &t2);

    // Only the master reports the results
    if (rank == 0)
    {
        // Print the result
        for (i = 2; i < n; i++)
            if (map[i] == 1)
                printf("%d ", i);
        printf("\n");

        // TODO: find the *largest* time among the workers
        //measure_print(&t1, &t2);
    }

    // Free resources
    MPI_Finalize();
    free(map);
    return EXIT_SUCCESS;
}
