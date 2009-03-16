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
#include "MknodCommand.h"

int MknodCommand::execute(Size nparams, char **params)
{
    dev_t dev;
    
    /* Fill in major/minor numbers. */
    dev.major = atoi(params[1]);
    dev.minor = atoi(params[2]);

    /* Attempt to create the file. */
    if (mknod(params[0], ZERO, dev) < 0)
    {
	printf("Failed to create '%s': %s\n",
		params[0], strerror(errno));
	return errno;
    }
    return 0;
}

INITOBJ(MknodCommand, mknodCmd, DEFAULT)
