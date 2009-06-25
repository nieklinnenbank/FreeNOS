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

void outputf(char *fmt, ...)
{
    char path[128], buf[1025];
    int fd;
    Error e;
    va_list av;
    
    /* Format the path. */
    va_start(av, fmt);
    vsnprintf(path, sizeof(path), fmt, av);
    va_end(av);

    /* Clear buffer. */
    memset(buf, 0, sizeof(buf));

    /* Attempt to open the file first. */
    if ((fd = open(path, ZERO)) < 0)
    {
        printf("Failed to open '%s': %s\r\n",
                path, strerror(errno));
	exit(EXIT_FAILURE);
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
		        path, strerror(errno));
		close(fd);
		exit(EXIT_FAILURE);
    
	    /* End of file. */
	    case 0:
		close(fd);
		return;
	
	    /* Print out results. */
	    default:
		buf[e] = 0;
		printf("%s", buf);
	        break;
	}
    }
    /* Not reached. */
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    DIR *d;
    struct dirent *dent;

    /* Print header. */
    printf("PID STATUS CMD\r\n");
    
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
	if (dent->d_name[0] != '.')
	{
	    printf("%s ", dent->d_name);
	    outputf("/proc/%s/status",  dent->d_name);
	    printf(" ");
	    outputf("/proc/%s/cmdline", dent->d_name);
	    printf("\r\n");
	}
    }
    /* Close it. */
    closedir(d);
    
    /* Done. */
    return EXIT_SUCCESS;
}
