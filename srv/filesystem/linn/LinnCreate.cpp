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

#include <BitMap.h>
#include <List.h>
#include <ListIterator.h>
#include <String.h>
#include <Types.h>
#include <Macros.h>
#include "LinnCreate.h"
#include "LinnSuperBlock.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>

LinnCreate::LinnCreate()
{
    prog      = ZERO;
    image     = ZERO;
    super     = ZERO;
    verbose   = false;
}

int LinnCreate::create(Size blocksize, Size blocknum)
{
    assert(image != ZERO);
    assert(prog  != ZERO);
    assert(blocknum >= 2);

    /* Allocate blocks. */
    blocks = new u8[blocksize * blocknum];
    memset(blocks, 0, blocksize * blocknum);

    /* Create a superblock. */
    super = (LinnSuperBlock *) (blocks + LINN_SUPER_OFFSET);
    super->magic0 = LINN_SUPER_MAGIC0;
    super->magic1 = LINN_SUPER_MAGIC1;
    super->majorRevision    = LINN_SUPER_MAJOR;
    super->minorRevision    = LINN_SUPER_MINOR;
    super->state            = LINN_SUPER_VALID;
    super->blockSize        = blocksize;
    super->blockAddrSize    = LINNCREATE_BLOCK_ADDR_SIZE;
    super->blocksPerGroup   = LINNCREATE_BLOCKS_PER_GROUP;
    super->inodesPerGroup   = LINNCREATE_INODES_PER_GROUP;
    super->inodesCount      = ZERO;
    super->blocksCount	    = blocknum;
    super->groupsCount      = ZERO;
    super->freeInodesCount  = ZERO;
    super->freeBlocksCount  = blocknum - 2;
    super->freeGroupsCount  = ZERO;
    super->creationTime     = time(NULL);
    super->mountTime	    = ZERO;
    super->mountCount	    = ZERO;
    super->lastCheck	    = ZERO;
    super->groupsTable	    = 2;

    /* Write the final image. */
    return writeImage();
}    

int LinnCreate::writeImage()
{
    FILE *fp;
    
    /* Open output image. */
    if ((fp = fopen(image, "w")) == NULL)
    {
	printf("%s: failed to fopen() `%s' for writing: %s\r\n",
		prog, image, strerror(errno));
	return EXIT_FAILURE;
    }
    /* Write all blocks at once. */
    if (fwrite(blocks, super->blockSize *
		      (super->blocksCount - super->freeBlocksCount), 1, fp) != 1)
    {
	printf("%s: failed to fwrite() `%s': %s\r\n",
		prog, image, strerror(errno));
	fclose(fp);
	return EXIT_FAILURE;
    }
    /* All done. */
    fclose(fp);
    return EXIT_SUCCESS;
}

void LinnCreate::setProgram(char *progName)
{
    this->prog = progName;
}

void LinnCreate::setImage(char *imageName)
{
    this->image = imageName;
}

void LinnCreate::setInput(char *inputName)
{
    this->input = inputName;
}

void LinnCreate::setExclude(char *pattern)
{
    this->excludes.insertTail(new String(pattern));
}

void LinnCreate::setVerbose(bool newVerbose)
{
    this->verbose = newVerbose;
}

int main(int argc, char **argv)
{
    LinnCreate fs;
    Size blocksize = LINNCREATE_BLOCK_SIZE;
    Size blocknum  = LINNCREATE_BLOCK_NUM;

    /* Verify command-line arguments. */
    if (argc < 3)
    {
	printf("usage: %s IMAGE DIRECTORY [OPTIONS...]\r\n"
	       "Creates a new Linnenbank FileSystem\r\n"
	       "\r\n"
	       "-h          Show this help message.\r\n"
	       "-v          Output verbose messages.\r\n"
	       "-e PATTERN  Exclude matching files from the created filesystem\r\n"
	       "-b SIZE     Specifies the blocksize.\r\n"
	       "-n COUNT    Specifies the maximum number of blocks.\r\n",
		argv[0]);
	return EXIT_FAILURE;
    }
    /* Process command-line arguments. */
    fs.setProgram(argv[0]);
    fs.setImage(argv[1]);
    fs.setInput(argv[2]);
    
    /* Process command-line options. */
    for (int i = 0; i < argc - 3; i++)
    {
	/* Exclude files matching the given pattern. */
	if (!strcmp(argv[i + 3], "-e") && i < argc - 4)
	{
	    fs.setExclude(argv[i + 4]);
	    i++;
	}
	/* Verbose output. */
	else if (!strcmp(argv[i + 3], "-v"))
	{
	    fs.setVerbose(true);
	}
	/* Block size. */
	else if (!strcmp(argv[i + 3], "-b") && i < argc - 4)
	{
	    blocksize = atoi(argv[i + 4]);
	    i++;
	}
	/* Maximum block count. */
	else if (!strcmp(argv[i + 3], "-n") && i < argc - 4)
	{
	    if ((blocknum = atoi(argv[i + 4])) < 2)
	    {
		printf("%s: block count must be >= 2\r\n",
			argv[0]);
		return EXIT_FAILURE;
	    }
	    i++;
	}
	/* Unknown argument. */
	else
	    printf("%s: unknown option `%s'\r\n",
		    argv[0], argv[i + 3]);
    }
    /* Create a new Linnenbank FileSystem. */
    return fs.create(blocksize, blocknum);
}
