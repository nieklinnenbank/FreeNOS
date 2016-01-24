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
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <Runtime.h>

int statFile(char *prog, char *file)
{
    struct stat st;

    /* Try to stat the file. */
    if ((stat(file, &st)) < 0)
    {
	printf("%s: failed to stat '%s': %s\r\n",
		prog, file, strerror(errno));
	return EXIT_FAILURE;
    }
    /* Output file statistics. */
    printf("File: %s\r\n", file);
    printf("Type: ");
    
    /* Print the right file type. */
    if (S_ISREG(st.st_mode))
	printf("Regular File\r\n");
    
    else if (S_ISDIR(st.st_mode))
	printf("Directory\r\n");
    
    else if (S_ISCHR(st.st_mode))
    {
	printf("Character Device\r\n");
	printf("Major ID: %u\r\n", st.st_dev.major);
	printf("Minor ID: %u\r\n", st.st_dev.minor);
    }
    else if (S_ISBLK(st.st_mode))
    {
	printf("Block Device\r\n");
	printf("Major ID: %u\r\n", st.st_dev.major);
	printf("Minor ID: %u\r\n", st.st_dev.minor);	
    }
    else
	printf("Unknown\r\n");

    printf("Mode: %u\r\n", st.st_mode);
    printf("Size: %u\r\n", st.st_size);   
    printf("Uid:  %u\r\n", st.st_uid);
    printf("Gid:  %u\r\n", st.st_gid);

    /* Success. */
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    int ret = EXIT_SUCCESS, result;

    /* Verify command-line arguments. */
    if (argc <= 1)
    {
	printf("usage: %s FILE ...\r\n",
		argv[0]);
	return EXIT_FAILURE;
    }    
    refreshMounts(0);

    /* Perform a stat for each file. */
    for (int i = 0; i < argc - 1; i++)
    {
	/* Stat the file immediately. */
	result = statFile(argv[0], argv[i + 1]);
	
	/* Update exit status, if needed. */
	if (result > ret)
	{
	    ret = result;
	}
    }    
    /* Success. */
    return ret;
}
