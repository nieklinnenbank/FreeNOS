/*
 * Copyright (C) 2009 Niek Linnenbank
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
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int sec = 0;

    /* Verify command-line arguments. */
    if (argc <= 1)
    {
	printf("usage: %s SECONDS\r\n", argv[0]);
	return EXIT_FAILURE;
    }

    /* Convert input to seconds */
    if ((sec = atoi(argv[1])) <= 0)
    {
        printf("%s: invalid sleep time '%s'\r\n", argv[0], argv[1]);
        return EXIT_FAILURE;
    }

    /* Sleep now */
    if (sleep(sec) != 0)
    {
        printf("%s: failed to sleep: %s\r\n", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }

    /* Done */
    return EXIT_SUCCESS;
}
