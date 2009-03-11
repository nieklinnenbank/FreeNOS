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
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include "DirListCommand.h"

int DirListCommand::execute(Size nparams, char **params)
{
    DIR *d;
    struct dirent *dent;
    
    /* Attempt to open the directory. */
    if (!(d = opendir(params[0])))
    {
	printf("Failed to open '%s': %s\n",
		params[0], strerror(errno));
	return errno;
    }
    /* Read directory. */
    while ((dent = readdir(d)))
    {
	printf("%s ", dent->d_name);
    }
    printf("\n");

    /* Close it. */
    closedir(d);
    
    /* Success. */
    return 0;
}

INITOBJ(DirListCommand, dirListCmd, DEFAULT)
