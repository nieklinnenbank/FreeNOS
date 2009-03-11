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

#include <utsname.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "UnameCommand.h"

int UnameCommand::execute(Size nparams, char **params)
{
    struct utsname info;

    /* Retrieve version information. */
    if (uname(&info) < 0)
    {
	printf("uname() failed: %s\n",
		strerror(errno));
	return errno;
    }
    /* System name. */
    if (nparams == 0 || !strcmp(params[0], "-s"))
    {
	printf("%s\n", info.sysname);
    }
    /* Everything. */
    else if (!strcmp(params[0], "-a"))
    {
        printf("%s %s %s %s %s\n",
                info.sysname,
                info.nodename,
                info.release,
                info.version,
                info.machine);
    }
    return 0;
}

INITOBJ(UnameCommand, unameCmd, NORMAL)
