/**
 * Simple implementation of Sieve's algorithm.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <String.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>

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

int main(int argc, char **argv)
{
    uint *map;
    String output;
    int n, k = 2, i, last, sqrt_of_n;

    // Check commandline arguments
    if (argc < 2)
    {
        printf("usage: %s <number> [--stdout]\n", argv[0]);
        return EXIT_FAILURE;
    }
    // Read max number
    n = atoi(argv[1]);
    sqrt_of_n = sqrt(n);

    // Try to allocate memory
    if ((map = (uint *) malloc(n * sizeof(uint))) == NULL)
    {
        printf("%s: malloc failed: %s", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }
    // Set to true
    for (i = 0; i < n; i++)
        map[i] = 1;

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
        i    = sqrt_of_n;
        last = n;

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
    // Print the result
    if (argc >= 3 && strcmp(argv[2], "--stdout") == 0)
    {
        for (i = 2; i < n; i++)
        {
            if (map[i] == 1)
                output << " " << i;
        }
        // Done
        output << "\r\n";
        write(1, *output, output.length());
    }
    return EXIT_SUCCESS;
}
