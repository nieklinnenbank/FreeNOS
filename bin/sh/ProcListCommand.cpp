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
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include "ProcListCommand.h"
#include "ShellCommand.h"

int ProcListCommand::execute(Size nparams, char **params)
{
    DIR *d;
    struct dirent *dent;
    
    /* Attempt to open the directory. */
    if (!(d = opendir("/proc/")))
    {
	printf("Failed to open '/proc/': %s\n",
		strerror(errno));
	return errno;
    }
    printf("PID STATUS CMD\n");
    
    /* Read directory. */
    while ((dent = readdir(d)))
    {
	printf("%s ", dent->d_name);
	catFmt("/proc/%s/status",  dent->d_name);
	printf(" ");
	catFmt("/proc/%s/cmdline", dent->d_name);
	printf("\n");
    }
    /* Close it. */
    closedir(d);
    return 0;
}

void ProcListCommand::catFmt(char *fmt, ...)
{
    char path[128];
    char *args[] = { path, 0 };
    ShellCommand *catCmd = 0;
    va_list av;
    
    /* Format the path. */
    va_start(av, fmt);
    vsnprintf(path, sizeof(path), fmt, av);
    va_end(av);
    
    /* Invoke cat. */
    if ((catCmd = ShellCommand::byName("cat")))
    {
	catCmd->execute(1, args);
    }
}

INITOBJ(ProcListCommand, procListCmd, DEFAULT)
