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

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "WriteCommand.h"

int WriteCommand::execute(Size nparams, char **params)
{
    int fd;

    /* Attempt to open the file first. */
    if ((fd = open(params[0], ZERO)) < 0)
    {
        printf("Failed to open '%s': %s\n",
                params[0], strerror(errno));
        return errno;
    }
    /* Write to the file. */
    if (write(fd, params[1], strlen(params[1])) < 0)
    {
	printf("Failed to write '%s': %s\n",
		params[0], strerror(errno));
	return errno;
    }
    /* Close the file. */
    close(fd);
    return 0;
}

INITOBJ(WriteCommand, writeCmd, DEFAULT)
