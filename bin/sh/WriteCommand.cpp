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
    char space = ' ', *newline = "\r\n";

    /* Attempt to open the file first. */
    if ((fd = open(params[0], O_WRONLY)) < 0)
    {
        printf("Failed to open '%s': %s\r\n",
                params[0], strerror(errno));
        return errno;
    }
    /* Write to the file. */
    for (Size i = 0; i < nparams - 1; i++)
    {
	/* Write the argument, and a whitespace. */
	if (write(fd, params[i + 1], strlen(params[i + 1])) < 0 ||
	    write(fd, &space, 1) < 0)
	{
	    printf("Failed to write '%s': %s\r\n",
		    params[0], strerror(errno));
	    return errno;
	}	
    }
    /* Newline. */
    write(fd, newline, 2);
    
    /* Close the file. */
    close(fd);
    return 0;
}
