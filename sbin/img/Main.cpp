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
#include <ExecutableFormat.h>

int main(int argc, char **argv)
{
    ExecutableFormat *format;
    MemoryRegion regions[16];
    int num;
    
    /* Verify command-line arguments. */
    if (argc < 2)
    {
	fprintf(stderr, "usage: %s [FILE]\n",
		argv[0]);
	return EXIT_FAILURE;
    }
    /* Attempt to parse the executable headers. */
    else if (!(format = ExecutableFormat::find(argv[1])))
    {
	fprintf(stderr, "%s: failed to read `%s': %s\n",
		argv[0], argv[1], strerror(errno));
	return EXIT_FAILURE;
    }
    /* Extract memory regions. */
    else if ((num = format->regions(regions, 16)) <= 0)
    {
	fprintf(stderr, "%s: failed to extract memory regions from `%s': %s\n",
		argv[0], argv[1], strerror(errno));
	return EXIT_FAILURE;
    }
    /* Debug out memory sections. */
    for (int i = 0; i < num; i++)
    {
	printf("%s[%u]: vaddr=%x size=%u\n",
		argv[1], i, (uint) regions[i].virtualAddress, regions[i].size);
    }
    /* Exit immediately. */
    return EXIT_SUCCESS;
}
