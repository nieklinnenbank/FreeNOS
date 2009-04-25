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
#include "CatCommand.h"

int CatCommand::execute(Size nparams, char **params)
{
    char buf[1025];
    int fd;
    Error e;

    /* Clear buffer. */
    memset(buf, 0, sizeof(buf));

    /* Attempt to open the file first. */
    if ((fd = open(params[0], ZERO)) < 0)
    {
        printf("Failed to open '%s': %s\r\n",
                params[0], strerror(errno));
        return errno;
    }
    /* Read contents. */
    while (1)
    {
	e = read(fd, buf, sizeof(buf) - 1);
	switch (e)
        {
	    /* Error occurred. */
	    case -1:
		printf("Failed to read '%s': %s\r\n",
		        params[0], strerror(errno));
		close(fd);
	        return errno;
    
	    /* End of file. */
	    case 0:
		close(fd);
		return ESUCCESS;
	
	    /* Print out results. */
	    default:
		buf[e] = 0;
		printf("%s", buf);
	        break;
	}
    }
    /* Not reached. */
    return ENOSUPPORT;
}

INITOBJ(CatCommand, catCmd, DEFAULT)
