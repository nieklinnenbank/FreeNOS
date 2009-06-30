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
#include <String.h>
#include "Ext2SuperBlock.h"
#include "Ext2Inode.h"
#include "Ext2Group.h"
#include "Ext2FileSystem.h"
#include "Ext2Create.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

Ext2Create::Ext2Create()
{
    prog  = ZERO;
    image = ZERO;
    super = ZERO;
    blockSize    = EXT2_MIN_BLOCK_SIZE;
    fragmentSize = EXT2_MIN_FRAG_SIZE;
}

int Ext2Create::create()
{
    assert(image != ZERO);
    assert(prog != ZERO);

    /* Allocate and initialize the superblock. */
    super = initSuperBlock();
    
    /* Write the final image. */
    return writeImage();
}    

Ext2InputFile * Ext2Create::addInputFile(char *inputFile, Ext2InputFile *parent)
{
    Ext2InputFile *file;
    struct stat st;
    
    /* Stat the input file. */
    if (stat(inputFile, &st) != 0)
    {
	printf("%s: failed to stat() `%s': %s\r\n",
		prog, inputFile, strerror(errno));
	exit(EXIT_FAILURE);
    }
    /* Debug out. */
    printf("%s mode=%x size=%lu userId=%u groupId=%u\r\n",
	    inputFile, st.st_mode, st.st_size, st.st_uid, st.st_gid);

    /* Create and clear a new input file. */
    file = new Ext2InputFile;
    memset(file, 0, sizeof(*file));

    /* Copy the filename. */
    strncpy(file->name, inputFile, EXT2_NAME_LEN);
    file->name[EXT2_NAME_LEN - 1] = 0;
    
    /* Fill in the inode. */
    file->inode.mode  = st.st_mode;
    file->inode.uid   = (le16) st.st_uid;
    file->inode.size  = st.st_size;
    file->inode.atime = st.st_atime;
    file->inode.ctime = st.st_mtime;
    file->inode.mtime = st.st_mtime;
    file->inode.dtime = st.st_mtime;
    file->inode.gid   = (le16) st.st_gid;
    file->inode.linksCount = st.st_nlink;
    file->inode.blocks     = (st.st_blocks * 512) / blockSize;
    file->inode.uidHigh    = (le16) st.st_uid >> 16;
    file->inode.gidHigh    = (le16) st.st_gid >> 16;

    /* Add it to the parent, or make it root. */
    if (parent)
	parent->childs.insertTail(file);
    else
	inputRoot = file;

    /* Update superblock. */
    super->inodesCount++;
    super->blocksCount += file->inode.blocks;
    
    /* All done. */
    return file;
}

int Ext2Create::readInput(char *directory, Ext2InputFile *parent)
{
    DIR *dir;
    Ext2InputFile *file;
    struct dirent *entry;
    char path[EXT2_NAME_LEN];
    
    /* Open the local directory. */
    if ((dir = opendir(directory)) == NULL)
    {
	return EXIT_FAILURE;
    }
    /* Add all entries. */
    while ((entry = readdir(dir)) != NULL)
    {
	/* Skip hidden. */
	if (entry->d_name[0] != '.')
	{
	    snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
	    file = addInputFile(path, parent);
	    
	    /* Traverse it in case of a directory. */
	    if (S_ISDIR(file->inode.mode))
	    {
		readInput(path, file);
	    }
	}
    }
    /* Cleanup. */
    closedir(dir);

    /* Done. */
    return EXIT_SUCCESS;
}

int Ext2Create::writeImage()
{
    FILE *fp;

    assert(image != ZERO);
    assert(prog != ZERO);
    
    /* Construct the input root. */
    addInputFile(input, ZERO);

    /* Add the input directory contents. */
    readInput(input, inputRoot);

    /* Debug out. */
    printf( "Writing Extended 2 FileSystem to `%s' (blocksize=%u inodes=%u)\r\n",
	     image, blockSize, super->inodesCount);

    /* Open output image file. */
    if ((fp = fopen(image, "w")) == NULL)
    {
	printf("%s: failed to fopen `%s': %s\r\n",
		prog, image, strerror(errno));
	return EXIT_FAILURE;
    }
    /* Seek to second block. */
    if (fseek(fp, blockSize, SEEK_SET) != 0)
    {
	printf("%s: failed to seek `%s' to %x: %s\r\n",
		prog, image, blockSize, strerror(errno));
	return EXIT_FAILURE;
    }
    /* Write superblock. */
    if (fwrite(super, sizeof(*super), 1, fp) != 1)
    {
	printf("%s: failed to fwrite `%s': %s\r\n",
		prog, image, strerror(errno));
	return EXIT_FAILURE;
    }
    /* Cleanup. */
    fclose(fp);
    delete super;
    
    /* All done. */
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

Ext2SuperBlock * Ext2Create::initSuperBlock()
{
    Ext2SuperBlock *sb = new Ext2SuperBlock;
    sb->inodesCount         = 0;
    sb->blocksCount         = 0;
    sb->reservedBlocksCount = ZERO;
    sb->freeBlocksCount     = 0;
    sb->freeInodesCount     = 0;
    sb->firstDataBlock      = 1;
    sb->log2BlockSize       = blockSize    >> 10;
    sb->log2FragmentSize    = fragmentSize >> 10;
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
    return sb;
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
	       "-b SIZE     Block size.\r\n"
	       "-n INODES   Number of inodes.\r\n",
		argv[0]);
	return EXIT_FAILURE;
    }
    /* Process command-line arguments. */
    fs.setProgram(argv[0]);
    fs.setImage(argv[1]);
    fs.setInput(argv[2]);
    
    /* Create a new Extended 2 FileSystem. */
    return fs.create();
}
