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
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <TerminalCodes.h>

int main(int argc, char **argv)
{
    DIR *d;
    struct dirent *dent;

    /* Verify command-line arguments. */
    if (argc <= 1)
    {
	printf("usage: %s DIRECTORY\r\n",
		argv[0]);
	return EXIT_FAILURE;
    }
    /* Attempt to open the directory. */
    if (!(d = opendir(argv[1])))
    {
	printf("%s: failed to open '%s': %s\r\n",
		argv[0], argv[1], strerror(errno));
	return EXIT_FAILURE;
    }
    /* Read directory. */
    while ((dent = readdir(d)))
    {
	/* Coloring. */
	if (dent->d_type == DT_DIR)
	    printf("%s", BLUE);
	else
	    printf("%s", WHITE);
	printf("%s ", dent->d_name);
    }
    printf("\r\n");

    /* Close it. */
    closedir(d);
    
    /* Success. */
    return EXIT_SUCCESS;
}
