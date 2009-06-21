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

#ifndef __LIBEXEC_ELF_H
#define __LIBEXEC_ELF_H

#include <List.h>
#include <Types.h>
#include <Factory.h>
#include "ExecutableFormat.h"
#include "ELFHeader.h"

/**   
 * @defgroup libexec_elf libexec (ELF)
 * @{   
 */

/**
 * Executable and Linkable Format (ELF).
 */
class ELF : public ExecutableFormat
{
    public:

	/**
	 * Class constructor.
	 * @param path Path on filesystem to the ELF executable.
	 * @param fd File descriptor of the ELF executable.
	 * @param header ELF header read from the file.
	 */
	ELF(const char *path, int fd, ELFHeader *header);

	/**
	 * Class destructor.
	 */
	~ELF();
    
	/**
	 * Reads out segments from the ELF program table.
	 * @param regions Memory regions to fill.
	 * @param max Maximum number of memory regions.
	 * @return Number of memory regions or an error code on error.
	 */
	int regions(MemoryRegion *regions, Size max);

	/**
	 * Lookup the program entry point.
	 * @return Program entry point.
	 */
	Address entry();

	/**
	 * Confirms if we understand the given format.
	 * @param path Path to the file to read.
	 * @return true on success and false on failure.
	 */
	static ExecutableFormat * detect(const char *path);

    private:

	/** File descriptor of the ELF executable. */
	int fd;
	
	/** ELF header. */
	ELFHeader header;
};

/**
 * @}
 */

#endif /* __LIBEXEC_ELF_H */
