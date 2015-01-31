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

#include <Arch/Memory.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include "ELF.h"

ELF::ELF(const char *p, int f, ELFHeader *h)
    : ExecutableFormat(p), fd(f)
{
    memcpy(&header, h, sizeof(header));
}

ELF::~ELF()
{
    close(fd);
}

ExecutableFormat * ELF::detect(const char *path)
{
    ELFHeader header;
    int fd;

    /* Open target file. */
    if ((fd = open(path, ZERO)) < 0)
    {
	return ZERO;
    }
    /* Read ELF header. */
    if (read(fd, (void *) &header, sizeof(header)) != sizeof(header))
    {
	close(fd);
	return ZERO;
    }    
    /* Verify ELF magic. */    
    if (header.ident[ELF_INDEX_MAGIC0] == ELF_MAGIC0 &&
	header.ident[ELF_INDEX_MAGIC1] == ELF_MAGIC1 &&
	header.ident[ELF_INDEX_MAGIC2] == ELF_MAGIC2 &&
	header.ident[ELF_INDEX_MAGIC3] == ELF_MAGIC3)
    {
	/* Only accept current, 32-bit ELF executable programs. */
	if (header.ident[ELF_INDEX_CLASS] == ELF_CLASS_32 &&
	    header.version == ELF_VERSION_CURRENT &&
	    header.type    == ELF_TYPE_EXEC)
	{
	    return new ELF(path, fd, &header);
	}
    }
    close(fd);
    return ZERO;
}

int ELF::regions(MemoryRegion *regions, Size max)
{
    ELFSegment segments[16];
    Size count = 0;
    
    /* Must be of the same sizes. */
    if (!(header.programHeaderEntrySize == sizeof(ELFSegment) &&
          header.programHeaderEntryCount < 16))
    {
	errno = ENOEXEC;
	return -1;
    }
    /* Point to the program header. */
    if (lseek(fd, header.programHeaderOffset, SEEK_SET) == -1)
    {
	return -1;
    }
    /* Read all segments. */
    if (read(fd, &segments,
	     sizeof(ELFSegment) * header.programHeaderEntryCount) < 0)
    {
	return -1;
    }
    /* Fill in the memory regions. */
    for (Size i = 0; i < max && i < header.programHeaderEntryCount; i++)
    {
	/* We are only interested in loadable segments. */
	if (segments[i].type != ELF_SEGMENT_LOAD)
	{
	    continue;
	}
	regions[i].virtualAddress = segments[i].virtualAddress;
	regions[i].size  = segments[i].memorySize;
	regions[i].flags = PAGE_RW;
	regions[i].data  = new u8[segments[i].memorySize];
	
	/* Read segment contents from file. */
	if (lseek(fd, segments[i].offset, SEEK_SET) == -1 ||
	    read (fd, regions[i].data, segments[i].fileSize) < 0)
	{
	    errno = ENOEXEC;
	    return -1;
	}
	/* Nulify remaining space. */
	if (segments[i].memorySize > segments[i].fileSize)
	{
	    memset(regions[i].data + segments[i].memorySize, 0,
		   segments[i].memorySize - segments[i].fileSize);
	}
	/* Increment counter. */
	count++;
    }
    /* All done. */
    errno = 0;
    return count;
}

Address ELF::entry()
{
    return header.entry;
}
