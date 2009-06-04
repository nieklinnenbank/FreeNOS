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
#include <Init.h>
#include "ExecutableFormat.h"

/** Number of bytes in the ELF identity field. */
#define ELF_NIDENT 16

/** Magic number byte 0. */
#define ELF_MAGIC0 0x7f

/** Magic number byte 1. */
#define ELF_MAGIC1 'E'

/** Magic number byte 2. */
#define ELF_MAGIC2 'L'

/** Magic number byte 3. */
#define ELF_MAGIC3 'F'

/**
 * Executable and Linkable Format (ELF).
 */
class ELF : public ExecutableFormat
{
    public:

	/**
	 * Class constructor.
	 */
	ELF();

	/**
	 * Class destructor.
	 */
	~ELF();
    
	/**
	 * Memory regions a program needs at runtime.
	 * @param regions Memory regions to fill.
	 * @param max Maximum number of memory regions.
	 * @return Number of memory regions or an error code on error.
	 */
	int regions(MemoryRegion *regions, Size max);

	/**
	 * Lookup the program entry point.
	 * @param buf Entry point is written here.
	 * @return Zero on success and an error code on error.
	 */
	int entry(Address *buf);

	/**
	 * Confirms if we understand the given format.
	 * @param path Path to the file to read.
	 * @return true on success and false on failure.
	 */
	static ExecutableFormat * detect(const char *path);

    private:

	REGISTER(formats, ELF, detect)
};



/**
 * Describes an ELF executable and must be placed at the beginning of executable programs.
 */
typedef struct ELFHeader
{
    /** Magic number and other info. */
    u8  ident[ELF_NIDENT];
    
    /** Object file type. */
    u16 type;
    
    /** Physical machine architecture. */
    u16 machine;
    
    /** Object file version. */
    u32 version;              
    
    /** Entry point virtual address. */
    u32 entry;
    
    /** Program header table file offset. */
    u32 programHeaderOffset;
    
    /* Section header table file offset */
    u32 sectionHeaderOffset;
    
    /* Processor-specific flags. */
    u16 flags;
    
    /** ELF header size in bytes. */
    u16 headerSize;
    
    /** Program header table entry size. */
    u16 programHeaderEntrySize;

    /** Program header table entry count. */
    u16 programHeaderEntryCount;
    
    /** Section header table entry size. */
    u16 sectionHeaderEntrySize;
    
    /** Section header table entry count. */
    u16 sectionHeaderEntryCount;

    /** Section header string table index. */
    u16 sectionHeaderStringsIndex;
}
ELFHeader;

#endif /* __LIBEXEC_ELF_H */
