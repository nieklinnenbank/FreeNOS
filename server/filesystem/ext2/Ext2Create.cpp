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
#include "Ext2SuperBlock.h"
#include "Ext2Inode.h"
#include "Ext2Group.h"
#include "Ext2FileSystem.h"
#include "Ext2Directory.h"
#include "Ext2Create.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>

Ext2Create::Ext2Create()
{
    prog      = ZERO;
    image     = ZERO;
    super     = ZERO;
    verbose   = false;
    blockSize = EXT2_MAX_BLOCK_SIZE;
}

int Ext2Create::create()
{
    assert(image != ZERO);
    assert(prog != ZERO);

    /* Allocate blocks. */
    blocks = new u8[blockSize * EXT2CREATE_NUM_BLOCKS];

    /* Initialize superblock and group descriptor. */
    initSuperBlock((Ext2SuperBlock *)(blocks + EXT2_SUPER_OFFSET));
    initGroup(BLOCKPTR(Ext2Group, 1));
    
    /* Fill the filesystem using a local directory. */
    readInput(input, ZERO);
    
    /* Update superblock and group fields. */
    super->blocksCount     = blockMap->markNext();
    super->freeInodesCount = inodeMap->getFree();
    super->freeBlocksCount = 0;
    group->freeInodesCount = inodeMap->getFree();
    group->freeBlocksCount = 0;
    
    /* Write the final image. */
    return writeImage();
}    

Ext2Inode * Ext2Create::createInode(char *inputFile, Size *number)
{
    Ext2Inode *inode;
    struct stat st;
    FILE *fp;
    bool reading = true;
    Size blockNr, *indirect = ZERO, numBlocks = ZERO;
    
    /* Stat the input file. */
    if (stat(inputFile, &st) != 0)
    {
        printf("%s: failed to stat() `%s': %s\r\n",
                prog, inputFile, strerror(errno));
        exit(EXIT_FAILURE);
    }
    /* Debug out. */
    if (verbose)
    printf("%s mode=%x size=%llu userId=%u groupId=%u\r\n",
            inputFile, (unsigned int) st.st_mode,
	    (long long unsigned)st.st_size, st.st_uid, st.st_gid);

    /* Grab a new inode, or use the given number. */
    if (*number)
    {
	inode  = BLOCKPTR(Ext2Inode, group->inodeTable);
	inode += *number - 1;
    }
    else
    {
	*number = inodeMap->markNext();
	inode   = BLOCKPTR(Ext2Inode, group->inodeTable);
	inode  += *number - 1;
    }
    /* Fill it. */
    inode->mode  = st.st_mode;
    inode->uid   = (le16) st.st_uid;
    inode->size  = st.st_size;
    inode->atime = st.st_atime;
    inode->ctime = st.st_mtime;
    inode->mtime = st.st_mtime;
    inode->dtime = ZERO;
    inode->gid   = (le16) st.st_gid;
    inode->linksCount = 1;
    inode->blocks     = ZERO;
    inode->uidHigh    = (le16) st.st_uid >> 16;
    inode->gidHigh    = (le16) st.st_gid >> 16;

    /* In case of a directory, return it now. */
    if (S_ISDIR(st.st_mode))
    {
	return inode;
    }
    /* Otherwise, add the contents of the file. Open it first. */
    if ((fp = fopen(inputFile, "r")) == NULL)
    {
	printf("%s: failed to fopen() `%s': %s\r\n",
	        prog, inputFile, strerror(errno));
	exit(EXIT_FAILURE);
    }
    /* Now read it's contents. */
    while (reading)
    {
	/* Fetch next block. */
	blockNr = blockMap->markNext();
    
	/* Read the block from file. */
	if (fread(BLOCKPTR(char, blockNr), blockSize, 1, fp) != 1)
	{
	    /* Did we hit the end of the file? */
	    if (feof(fp))
		reading = false;
	    else
	    {
		blockMap->unmark(blockNr);
		break;
	    }
	}
	/* Insert block into inode. */
	if (numBlocks < EXT2_NDIR_BLOCKS)
	{
	    inode->block[numBlocks] = blockNr;
	}
	/* Indirect block. */
	else if (numBlocks < EXT2_ADDR_PER_BLOCK(super))
	{
	    if (indirect == ZERO)
	    {
		inode->block[EXT2_IND_BLOCK] = blockMap->markNext();
		inode->blocks += blockSize / 512;
		indirect = BLOCKPTR(Size, inode->block[EXT2_IND_BLOCK]);
	    }	
	    indirect[numBlocks - EXT2_NDIR_BLOCKS] = blockNr;
	}
	/* We do not support double/triple indirect yet. */
	else
	{
	    printf("%s: double/triple indirect blocks not supported: `%s'\r\n",
		    prog, inputFile);
	    exit(EXIT_FAILURE);
	}
	/* Increment block count. */
	inode->blocks += blockSize / 512;
	numBlocks++;
    }
    /* All done. */
    fclose(fp);
    return inode;
}

Size Ext2Create::readInput(char *directory, Size parent)
{
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    char path[EXT2_NAME_LEN];
    Ext2Inode *inode = ZERO;
    Ext2DirectoryEntry *dent, *dprev = ZERO;
    Size inodeNumber = ZERO, childInodeNum = ZERO;
    Address last;
    bool skip = false;

    /* Make a root inode? */
    if (!parent)
	inodeNumber = EXT2_ROOT_INO;

    /* Create an Inode first. */
    inode = createInode(directory, &inodeNumber);
    inode->blocks     = blockSize / 512;
    inode->block[0]   = blockMap->markNext();
    inode->size       = blockSize;
    inode->linksCount = 2;
    dent = BLOCKPTR(Ext2DirectoryEntry, inode->block[0]);
    last = (Address) (dent) + blockSize;
    
    /* Create '.' and '..' directory entries. */
    EXT2_CREATE_DIRENT(".",  inodeNumber);
    EXT2_CREATE_DIRENT("..", parent ? parent : inodeNumber);
    
    /* Open the local directory. */
    if ((dir = opendir(directory)) == NULL)
    {
	return EXIT_FAILURE;
    }
    /* Add all entries. */
    while ((entry = readdir(dir)) != NULL)
    {
	/* Reset. */
	skip = false;
	
	/* Skip excludes. */
	for (ListIterator<String> i(&excludes); i.hasNext(); i++)
	{
	    if (i.current()->match(entry->d_name, **i.current()))
	    {
		skip = true;
		break;
	    }
	}
	/* Skip hidden. */
	if (entry->d_name[0] != '.' && !skip)
	{
	    /* Construct the full path. */
	    snprintf(path, sizeof(path), "%s/%s",
		     directory, entry->d_name);
	    
	    /* Traverse it in case of a directory. */
	    stat(path, &st);
	    if (S_ISDIR(st.st_mode))
	    {
		childInodeNum = readInput(path, inodeNumber);
		inode->linksCount++;
	    }
    	    /* Create a new Inode for the file. */
	    else
	    {
		createInode(path, &childInodeNum);
	    }
	    EXT2_CREATE_DIRENT(basename(path), childInodeNum);
	    childInodeNum = ZERO;
	}
    }
    /* Cleanup. */
    closedir(dir);

    /* Done. */
    return inodeNumber;
}

int Ext2Create::writeImage()
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
    if (fwrite(blocks, blockSize * super->blocksCount, 1, fp) != 1)
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

void Ext2Create::setProgram(char *progName)
{
    this->prog = progName;
}

void Ext2Create::setImage(char *imageName)
{
    this->image = imageName;
}

void Ext2Create::setInput(char *inputName)
{
    this->input = inputName;
}

void Ext2Create::setExclude(char *pattern)
{
    this->excludes.insertTail(new String(pattern));
}

void Ext2Create::setVerbose(bool newVerbose)
{
    this->verbose = newVerbose;
}

void Ext2Create::initSuperBlock(Ext2SuperBlock *sb)
{
    super = sb;
    sb->inodesCount         = EXT2CREATE_INODES_PER_GROUP;
    sb->blocksCount         = 3;
    sb->reservedBlocksCount = ZERO;
    sb->freeBlocksCount     = 0;
    sb->freeInodesCount     = 0;
    sb->firstDataBlock      = 0;
    sb->log2BlockSize       = blockSize >> 11;
    sb->log2FragmentSize    = blockSize >> 11;
    sb->blocksPerGroup      = EXT2CREATE_BLOCKS_PER_GROUP;
    sb->fragmentsPerGroup   = EXT2CREATE_FRAGS_PER_GROUP;
    sb->inodesPerGroup      = EXT2CREATE_INODES_PER_GROUP;
    sb->mountTime	    = ZERO;
    sb->writeTime	    = ZERO;
    sb->mountCount	    = ZERO;
    sb->maximumMountCount   = 32;
    sb->magic               = EXT2_SUPER_MAGIC;
    sb->state               = EXT2_VALID_FS;
    sb->errors              = EXT2_ERRORS_CONTINUE;
    sb->minorRevision       = ZERO;
    sb->lastCheck           = ZERO;
    sb->checkInterval       = 0;
    sb->creatorOS           = EXT2_OS_FREENOS;
    sb->majorRevision       = EXT2_CURRENT_REV;
    sb->defaultReservedUid  = ZERO;
    sb->defaultReservedGid  = ZERO;
}

void Ext2Create::initGroup(Ext2Group *grp)
{
    /* Initialize the group descriptor. */
    group = grp;
    group->blockBitmap = 2;
    group->inodeBitmap = 3;
    group->inodeTable  = 4;
    group->freeBlocksCount = 0;
    group->freeInodesCount = super->inodesPerGroup;
    group->usedDirsCount   = ZERO;

    /* Create BitMap instances. */
    blockMap = new BitMap(BLOCKPTR(u8, 2),
			  super->blocksPerGroup);
    inodeMap = new BitMap(BLOCKPTR(u8, 3),
			  super->inodesPerGroup);

    /* Clear them. */
    blockMap->clear();
    inodeMap->clear();

    /* Mark the appropriate blocks used. */
    blockMap->markRange(0, 4);
    blockMap->markRange(4,
			super->inodesPerGroup / (blockSize / sizeof(Ext2Inode)));

    /* Mark special Inodes. */
    inodeMap->markRange(0, EXT2_GOOD_OLD_FIRST_INO);
}

int main(int argc, char **argv)
{
    Ext2Create fs;

    /* Verify command-line arguments. */
    if (argc < 3)
    {
	printf("usage: %s IMAGE DIRECTORY [OPTIONS...]\r\n"
	       "Creates a new Extended 2 FileSystem\r\n"
	       "\r\n"
	       "-h          Show this help message.\r\n"
	       "-e PATTERN  Exclude matching files from the created filesystem\r\n"
	       "-v          Output verbose messages.\r\n",
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
	/* Unknown argument. */
	else
	    printf("%s: unknown option `%s'\r\n",
		    argv[0], argv[i + 3]);
    }
    /* Create a new Extended 2 FileSystem. */
    return fs.create();
}
