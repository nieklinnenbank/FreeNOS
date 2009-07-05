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

#include <Types.h>
#include "LinnSuperBlock.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

char * timeString(u64 timestamp)
{
    char *buff = (char *) malloc(128);
    struct tm *tm;
    
    /* Check for zero timestamps. */
    if (!timestamp)
    {
	return strdup("Never");
    }
    /* Fill in the time struct. */
    tm = gmtime((const time_t *) &timestamp);
    
    /* Format time. */
    if (!strftime(buff, 128, "%F %T", tm))
    {
	return strdup("???");
    }
    /* Done. */
    else
	return buff;
}

void usage(char *prog)
{
    printf("usage: %s FILE [OPTIONS...]\r\n"
	   "Displays information of a Linnenbank Filesystem\r\n"
	   "\r\n"
	   "-h          Show this help message.\r\n"
	   "-v          Output verbose messages.\r\n",
	    prog);
}

int main(int argc, char **argv)
{
    LinnSuperBlock super;
    bool verbose = false;
    FILE *fp;

    /* Verify command-line arguments. */
    if (argc < 2)
    {
	usage(argv[0]);
	return EXIT_FAILURE;
    }
    /* Process command-line options. */
    for (int i = 0; i < argc - 2; i++)
    {
	/* Show help. */
	if (!strcmp(argv[i + 2], "-h"))
	{
	    usage(argv[0]);
	    return EXIT_SUCCESS;
	}
	/* Verbose output. */
	else if (!strcmp(argv[i + 2], "-v"))
	{
	    verbose = true;
	}
	/* Unknown argument. */
	else
	    printf("%s: unknown option `%s'\r\n",
		    argv[0], argv[i + 2]);
    }
    /* Attempt to open the given file. */
    if ((fp = fopen(argv[1], "r")) == NULL)
    {
	printf("%s: failed to fopen() `%s': %s\n",
		argv[0], argv[1], strerror(errno));
	return EXIT_FAILURE;
    }
    /* Seek to correct offset. */
    if (fseek(fp, LINN_SUPER_OFFSET, SEEK_SET) == -1)
    {
	printf("%s: failed to fseek() to %x in `%s': %s\n",
		argv[0], LINN_SUPER_OFFSET, argv[1], strerror(errno));
	return EXIT_FAILURE;
    }
    /* Read the superblock. */
    if (fread(&super, sizeof(super), 1, fp) != 1)
    {
	printf("%s: failed to fread() superblock from `%s': %s\n",
		argv[0], argv[1], ferror(fp) ? strerror(errno) : "End of file");
	return EXIT_FAILURE;
    }
    /* Verify magic. */
    if (super.magic0 != LINN_SUPER_MAGIC0 ||
        super.magic1 != LINN_SUPER_MAGIC1)
    {
	printf("%s: `%s' is not a LinnFS filesystem (magic mismatch)\n",
		argv[0], argv[1]);
	return EXIT_FAILURE;
    }
    /* Dump filesystem information. */
    printf( "LinnSuperBlock\n"
	    "[\n"
	    "   magic0          = %x\n"
	    "   magic1          = %x\n"
	    "   majorRevision   = %u\n"
	    "   minorRevision   = %u\n"
	    "   state           = %x\n"
	    "   blockSize       = %u\n"
	    "   blockAddrSize   = %u (%d bit)\n"
	    "   blocksPerGroup  = %u\n"
	    "   inodesPerGroup  = %u\n"
	    "   inodesCount     = %llu\n"
	    "   blocksCount     = %llu\n"
	    "   groupsCount     = %llu\n"
	    "   freeBlocksCount = %llu\n"
	    "   freeInodesCount = %llu\n"
	    "   freeGroupsCount = %llu\n"
	    "   creationTime    = %s\n"
	    "   mountTime       = %s\n"
	    "   mountCount      = %u\n"
	    "   lastCheck       = %s\n"
	    "   groupsTable     = %llu\n"
	    "]\n",
	    super.magic0, super.magic1,
	    super.majorRevision, super.minorRevision,
	    super.state,  super.blockSize,
	    super.blockAddrSize, super.blockAddrSize * 8,
	    super.blocksPerGroup, super.inodesPerGroup, super.inodesCount,
	    super.blocksCount, super.groupsCount,
	    super.freeBlocksCount, super.freeInodesCount,
	    super.freeGroupsCount, timeString(super.creationTime),
	    timeString(super.mountTime), super.mountCount,
	    timeString(super.lastCheck), super.groupsTable);
    
    /* Cleanup and terminate. */
    fclose(fp);
    return EXIT_FAILURE;
}
