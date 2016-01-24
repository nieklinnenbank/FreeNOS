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
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <Runtime.h>

int remove(char *prog, char *path)
{
    int ret;

    if ((ret = unlink(path)) < 0)
    {
	printf("%s: failed to unlink '%s': %s\r\n",
		prog, path, strerror(errno));
	return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    int ret = EXIT_SUCCESS, result;

    /* Verify command-line arguments. */
    if (argc < 2)
    {
	printf("usage: %s FILE1 FILE2 ...\r\n",
		argv[0]);
	return EXIT_FAILURE;
    }
    refreshMounts(0);

    /* Delete all given files. */
    for (int i = 0; i < argc - 1; i++)
    {
	/* Remove. */
	result = remove(argv[0], argv[i + 1]);

	/* Update exit code if needed. */
	if (result > ret)
	{
	    ret = result;
	}
    }
    /* Done. */
    return ret;
}
