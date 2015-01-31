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
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <Types.h>
#include <Macros.h>

void readfile(char *buf, Size size, char *fmt, ...)
{
    char path[128];
    int fd;
    Error e;
    va_list av;
    
    /* Format the path. */
    va_start(av, fmt);
    vsnprintf(path, sizeof(path), fmt, av);
    va_end(av);

    /* Attempt to open the file first. */
    if ((fd = open(path, ZERO)) < 0)
    {
        printf("Failed to open '%s': %s\r\n",
                path, strerror(errno));
	exit(EXIT_FAILURE);
    }
    /* Read contents. */
    switch ((e = read(fd, buf, size)))
    {
        /* Error occurred. */
	case -1:
	    printf("Failed to read '%s': %s\r\n",
	            path, strerror(errno));
	    close(fd);
	    exit(EXIT_FAILURE);
    
	/* End of file. */
	case 0:
	    close(fd);
	    return;
	
	/* Success. */
	default:
	    buf[e] = ZERO;
	    return;
    }
}

int main(int argc, char **argv)
{
    DIR *d;
    struct dirent *dent;
    char status[11];
    char cmdline[64];

    /* Print header. */
    printf("PID   STATUS     CMD\r\n");
    
    /* Attempt to open the directory. */
    if (!(d = opendir("/proc")))
    {
	printf("Failed to open '/proc': %s\r\n",
		strerror(errno));
	return errno;
    }
    /* Read directory. */
    while ((dent = readdir(d)))
    {
	if (dent->d_name[0] != '.' && dent->d_name[0] >= '0' && dent->d_name[0] <= '9')
	{
	    /* Read the process' status. */
	    readfile(status,  sizeof(status),
		    "/proc/%s/status",  dent->d_name);
	    readfile(cmdline, sizeof(cmdline),
		    "/proc/%s/cmdline", dent->d_name);

	    /* Output a line. */	
	    printf("%5s %10s %32s\r\n",
		    dent->d_name, status, cmdline);
	}
    }
    /* Close it. */
    closedir(d);
    
    /* Done. */
    return EXIT_SUCCESS;
}
