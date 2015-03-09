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
#include <TerminalCodes.h>

int run_test(char *path)
{
    int status;
    char *argv[2];

    argv[0] = path;
    argv[1] = 0;

#ifdef __HOST__
    system(path);
#else
    pid_t pid = forkexec(path, (const char **) argv);
    waitpid(pid, &status, 0);
#endif

    return status;
}

int run_tests(char **argv, char *path)
{
    DIR *d;
    struct dirent *dent;
    struct stat st;
    char tmp[255];

    /* Attempt to open the target directory. */
    if (!(d = opendir(path)))
    {
	printf("%s: failed to open '%s': %s\r\n",
		argv[0], path, strerror(errno));
	return EXIT_FAILURE;
    }
    /* Read directory. */
    while ((dent = readdir(d)))
    {
        String str(dent->d_name);
        snprintf(tmp, sizeof(tmp), "%s/%s", path, dent->d_name);

	/* Coloring. */
	switch (dent->d_type)
	{
	    case DT_DIR:
                if (dent->d_name[0] != '.')
                {
                    run_tests(argv, tmp);
                }
                break;
	
	    case DT_REG:
                /* Check if it is a test, if yes execute and wait */
                if (str.endsWith((const char *)"Test"))
                {
                    run_test(tmp);

                }
                break;

	    default:
                break;
	}
    }
    /* Close it. */
    closedir(d);

    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    char path[255];

    /* Grab command-line arguments, if any */
    if (argc > 1)
    {
	strncpy(path, argv[1], sizeof(path));
        path[254] = 0;
    }
    else
    {
        strncpy(path, dirname(argv[0]), sizeof(path));
        path[254] = 0;
    }
    /* Run all tests in the given directory, recursively */
    int ret = run_tests(argv, path);
    
    return ret;
}
