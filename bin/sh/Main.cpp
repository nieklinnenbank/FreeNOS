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

#include <FreeNOS/System.h>
#include "Shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char **argv)
{
    Shell sh;
    FILE *fp;
    struct stat st;
    char *contents;

    /* Verify command-line arguments. */
    if (argc >= 2)
    {
	/* Execute commands in each file. */
	for (int i = 0; i < argc - 1; i++)
	{
	    /* Query the file size. */
	    if (stat(argv[i + 1], &st) != 0)
	    {
		printf("%s: failed to stat() `%s': %s\r\n",
			argv[0], argv[i + 1], strerror(errno));
		continue;
	    }
	    /* Open file. */
	    if ((fp = fopen(argv[i + 1], "r")) == NULL)
	    {
		printf("%s: failed to fopen() `%s': %s\r\n",
			argv[0], argv[i + 1], strerror(errno));
		continue;
	    }
	    /* Allocate buffer storage. */
	    contents = new char[st.st_size];
	    
	    /* Read the entire file into memory. */
	    if (fread(contents, st.st_size, 1, fp) != (size_t) st.st_size)
	    {
		printf("%s: failed to fread() `%s': %s\r\n",
			argv[0], argv[i + 1], strerror(errno));
		fclose(fp);
		continue;
	    }
	    /* Parse it into lines. */
	    List<String> lines = String(contents).split('\n');
	    
	    /* Execute each command. */
	    for (ListIterator<String> i(lines); i.hasCurrent(); i++)
	    {
		sh.executeInput((char *) *i.current());
	    }
	    /* Cleanup. */
	    delete contents;
	    fclose(fp);
	}
    }
    /* Run an interactive Shell. */
    else
    {
        /* Show the user where to get help. */
        printf( "\r\n"
                "Entering interactive Shell. Type 'help' for the command list.\r\n"
                "\r\n");

        /* Begin loop. */
        return sh.run();
    }
    /* Success. */
    return EXIT_SUCCESS;
}
