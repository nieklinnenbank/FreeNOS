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
#include <sys/utsname.h>

int main(int argc, char **argv)
{
    struct utsname info;

    /* Retrieve version information. */
    if (uname(&info) < 0)
    {
	printf("%s: uname() failed: %s\r\n",
		argv[0], strerror(errno));
	return errno;
    }
    /* System name. */
    if (argc <= 1 || !strcmp(argv[1], "-s"))
    {
	printf("%s\r\n", info.sysname);
    }
    /* Everything. */
    else if (argc == 2 && !strcmp(argv[1], "-a"))
    {
        printf("%s %s %s %s %s\r\n",
                info.sysname,
                info.nodename,
                info.release,
                info.version,
                info.machine);
    }
    return EXIT_SUCCESS;
}
