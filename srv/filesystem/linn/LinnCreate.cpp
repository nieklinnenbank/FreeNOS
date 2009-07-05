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
#include "LinnGroup.h"
#include "LinnInode.h"
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

int LinnCreate::create(Size blockSize, Size blockNum, Size inodeNum)
{
    assert(image != ZERO);
    assert(prog  != ZERO);
    assert(blockNum >= 2);
    assert(inodeNum > 0);

    /* Allocate blocks. */
    blocks = new u8[blockSize * blockNum];
    memset(blocks, 0, blockSize * blockNum);

    /* Create a superblock. */
    super = (LinnSuperBlock *) (blocks + LINN_SUPER_OFFSET);
    super->magic0 = LINN_SUPER_MAGIC0;
    super->magic1 = LINN_SUPER_MAGIC1;
    super->majorRevision    = LINN_SUPER_MAJOR;
    super->minorRevision    = LINN_SUPER_MINOR;
    super->state            = LINN_SUPER_VALID;
    super->blockSize        = blockSize;
    super->blocksPerGroup   = LINNCREATE_BLOCKS_PER_GROUP;
    super->inodesCount      = inodeNum;
    super->blocksCount	    = blockNum;
    super->inodesPerGroup   = super->inodesCount / LINNGROUP_COUNT(super);
    super->freeInodesCount  = super->inodesCount;
    super->freeBlocksCount  = blockNum - 2;
    super->creationTime     = time(NULL);
    super->mountTime	    = ZERO;
    super->mountCount	    = ZERO;
    super->lastCheck	    = ZERO;
    super->groupsTable	    = 2;

    /* Allocate LinnGroups. */
    for (Size i = 0; i < LINNGROUP_COUNT(super); i++)
    {
	/* Point to the correct LinnGroup. */
	LinnGroup *group = BLOCKPTR(LinnGroup, 2) + i;

	/* Fill the group. */
	group->freeBlocksCount = super->blocksPerGroup;
	group->freeInodesCount = super->inodesPerGroup;
	group->blockMap        = BLOCKS(super, LINNGROUP_NUM_BLOCKMAP(super));
	group->inodeMap        = BLOCKS(super, LINNGROUP_NUM_INODEMAP(super));
	group->inodeTable      = BLOCKS(super, LINNGROUP_NUM_INODETAB(super));
	
	/* Increment counter. */
	super->freeBlocksCount -= LINNGROUP_NUM_BLOCKMAP(super) +
				  LINNGROUP_NUM_INODEMAP(super) +
				  LINNGROUP_NUM_INODETAB(super);
    }
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
    Size blockSize = LINNCREATE_BLOCK_SIZE;
    Size blockNum  = LINNCREATE_BLOCK_NUM;
    Size inodeNum  = LINNCREATE_INODE_NUM;

    /* Verify command-line arguments. */
    if (argc < 2)
    {
	printf("usage: %s IMAGE [OPTIONS...]\r\n"
	       "Creates a new Linnenbank FileSystem\r\n"
	       "\r\n"
	       " -h           Show this help message.\r\n"
	       " -v           Output verbose messages.\r\n"
	       " -d DIRECTORY Insert files from the given directory into the image\r\n"
	       " -e PATTERN   Exclude matching files from the created filesystem\r\n"
	       " -b SIZE      Specifies the blocksize in bytes.\r\n"
	       " -n COUNT     Specifies the maximum number of blocks.\r\n"
	       " -i COUNT     Specifies the number of inodes to allocate.\r\n",
		argv[0]);
	return EXIT_FAILURE;
    }
    /* Process command-line arguments. */
    fs.setProgram(argv[0]);
    fs.setImage(argv[1]);
    
    /* Process command-line options. */
    for (int i = 0; i < argc - 2; i++)
    {
	/* Exclude files matching the given pattern. */
	if (!strcmp(argv[i + 2], "-e") && i < argc - 3)
	{
	    fs.setExclude(argv[i + 3]);
	    i++;
	}
	/* Verbose output. */
	else if (!strcmp(argv[i + 2], "-v"))
	{
	    fs.setVerbose(true);
	}
	/* Input directory. */
	else if (!strcmp(argv[i + 2], "-d") && i < argc - 3)
	{
	    fs.setInput(argv[i + 3]);
	    i++;
	}
	/* Block size. */
	else if (!strcmp(argv[i + 2], "-b") && i < argc - 3)
	{
	    blockSize = atoi(argv[i + 3]);
	    i++;
	}
	/* Maximum block count. */
	else if (!strcmp(argv[i + 2], "-n") && i < argc - 3)
	{
	    if ((blockNum = atoi(argv[i + 3])) < 2)
	    {
		printf("%s: block count must be >= 2\r\n",
			argv[0]);
		return EXIT_FAILURE;
	    }
	    i++;
	}
	/* Inode count. */
	else if (!strcmp(argv[i + 2], "-i") && i < argc - 3)
	{
	    if ((inodeNum = atoi(argv[i + 3])) < 1)
	    {
		printf("%s: inode count must be >= 1\r\n",
			argv[0]);
		return EXIT_FAILURE;
	    };
	    i++;
	}
	/* Unknown argument. */
	else
	{
	    printf("%s: unknown option `%s'\r\n",
		    argv[0], argv[i + 2]);
	    return EXIT_FAILURE;
	}
    }
    /* Create a new Linnenbank FileSystem. */
    return fs.create(blockSize, blockNum, inodeNum);
}
