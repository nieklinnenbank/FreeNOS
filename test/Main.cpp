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

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <String.h>
#include <HashTable.h>
#include <HashIterator.h>
#include <Integer.h>
#include <TerminalCodes.h>

int run_test(char *path)
{
    int status;
    char *argv[2];

    argv[0] = path;
    argv[1] = 0;

#ifdef __HOST__
    status = system(path);
    if (status >= 0)
    status = WEXITSTATUS(status);
#else
    pid_t pid = forkexec(path, (const char **) argv);
    waitpid(pid, &status, 0);
#endif

    return status;
}

int run_tests(char **argv, char *path, HashTable<String, Integer<int> > *results, int *failures)
{
    DIR *d;
    struct dirent *dent;
    struct stat st;
    char tmp[255];
    int r;

    // Attempt to open the target directory.
    if (!(d = opendir(path)))
    {
    printf("%s: failed to open '%s': %s\r\n",
        argv[0], path, strerror(errno));
    return EXIT_FAILURE;
    }
    // Read directory.
    while ((dent = readdir(d)))
    {
        String str(dent->d_name);
        snprintf(tmp, sizeof(tmp), "%s/%s", path, dent->d_name);

        // Coloring.
        switch (dent->d_type)
        {
            case DT_DIR:
                if (dent->d_name[0] != '.')
                {
                    run_tests(argv, tmp, results, failures);
                }
                break;
    
            case DT_REG:
                // Check if it is a test, if yes execute and wait
                if (str.endsWith((const char *)"Test"))
                {
                    r = run_test(tmp);
                    results->insert(new String(strdup(tmp)), new Integer<int>(r));
                    if (r != 0)
                        (*failures)++;
                }
                break;

            default:
                break;
        }
    }
    // Close it.
    closedir(d);

    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    char path[255], tmp[255];
    int iterations = 1;

    // Grab command-line arguments, if any
    if (argc > 1)
    {
        strncpy(path, argv[1], sizeof(path));
        path[254] = 0;
    }
    else
    {
        strncpy(tmp, argv[0], sizeof(tmp));
        strncpy(path, dirname(tmp), sizeof(path));
        path[254] = 0;
    }
    if (argc > 2)
        iterations = atoi(argv[2]);

    for (int i = 0; i < iterations; i++)
    {
        // Run all tests in the given directory, recursively
        HashTable<String, Integer<int> > results;
        int failed = 0;
        int ret = run_tests(argv, path, &results, &failed);
        printf("%s: ", argv[0]);

        if (iterations > 1)
            printf("iteration %d: ", i+1);

        if (failed != 0)
            printf("%sFAIL%s", RED, WHITE);
        else
            printf("%sOK%s", GREEN, WHITE);


        printf("   (%d passed %d failed %d total)\r\n",
        results.count() - failed, failed, results.count());

        // Print the failed tests
        for (HashIterator<String, Integer<int> > i(&results); i.hasNext(); i++)
        {
            String *testname = i.key();
            int fails = **i.current();

            if (fails)
                printf("  %s: %d failures\r\n", **testname , fails);
        }
        if (failed)
            return failed;
    }
    return EXIT_SUCCESS;
}
