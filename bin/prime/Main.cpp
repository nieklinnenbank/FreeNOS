/**
 * Simple implementation of Sieve's algorithm.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <String.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    Size i, j, num;
    uint *map;
    String output;
    //struct timeval t1, t2;

    // Check commandline arguments
    if (argc < 2)
    {
        printf("usage: %s <number>\n", argv[0]);
        return EXIT_FAILURE;
    }
    // Read max number
    num = (Size) atoi(argv[1]);

    // Try to allocate memory
    if ((map = (uint *) malloc(num * sizeof(uint))) == NULL)
    {
        printf("%s: malloc failed: %s", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }
    // Set to true
    memset(map, 1, num * sizeof(uint));

    //measure_start(&t1, &t2);

    // Next is a prime
    for (i = 2; i < num; i++)
    {
        // Prime number?
        if (map[i])
        {
            output << i << " ";

            // Mask off all multiples
            for (j = i; j < num; j++)
            {
                if (!(j % i))
                    map[j] = 0;
            }
        }
    }
    // Done
    output << "\n";
    write(1, *output, output.length());

    //measure_stop(&t1, &t2);
    //measure_print(&t1, &t2);
    return EXIT_SUCCESS;
}
