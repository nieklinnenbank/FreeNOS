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

#include <unistd.h>
#include <fcntl.h>
#include <Init.h>
#include "ELF.h"

ELF::ELF()
{
}

ELF::~ELF()
{
}

ExecutableFormat * ELF::detect(const char *path)
{
    ELFHeader header;
    int fd;
    
    /* Open target file. */
    if ((fd = open(path, ZERO)) < 0)
    {
	return false;
    }
    /* Read ELF header. */
    if (read(fd, (void *) &header, sizeof(header)) != sizeof(header))
    {
	close(fd);
	return ZERO;
    }    
    /* Verify ELF magic. */
    close(fd);
    
    if (header.ident[0] == ELF_MAGIC0 && header.ident[1] == ELF_MAGIC1 &&
	header.ident[2] == ELF_MAGIC2 && header.ident[3] == ELF_MAGIC3)
    {
	return new ELF;
    }
    else
	return ZERO;
}

int ELF::regions(MemoryRegion *regions, Size max)
{
    return -1;
}

int ELF::entry(Address *buf)
{
    return -1;
}
