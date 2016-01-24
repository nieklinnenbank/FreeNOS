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
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <Runtime.h>

int main(int argc, char **argv)
{
    dev_t dev;

    /* Verify command-line arguments. */
    if (argc < 5)
    {
	printf("usage: %s FILE TYPE MAJOR MINOR\r\n",
		argv[0]);
	return EXIT_FAILURE;
    }
    refreshMounts(0);

    /* Fill in major/minor numbers. */
    dev.major = atoi(argv[3]);
    dev.minor = atoi(argv[4]);

    /* Attempt to create the file. */
    if (mknod(argv[1], S_IFCHR, dev) < 0)
    {
	printf("%s: failed to create '%s': %s\r\n",
		argv[0], argv[1], strerror(errno));
	return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
