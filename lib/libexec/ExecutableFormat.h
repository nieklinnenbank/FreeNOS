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

#ifndef __LIBEXEC_EXECUTABLEFORMAT_H
#define __LIBEXEC_EXECUTABLEFORMAT_H
#ifndef __ASSEMBLER__

#include <List.h>
#include <Types.h>

/**  
 * @defgroup libexec libexec
 * @{  
 */

/**
 * Abstracts a memory region read from a format.
 */
typedef struct MemoryRegion
{
    /**
     * Constructor.
     */
    MemoryRegion() : virtualAddress(0), size(0), data(0)
    {
    }
    
    /**
     * Destructor.
     */
    ~MemoryRegion()
    {
	if (data)
	    delete data;
    }

    /** Beginning of the region. */
    Address virtualAddress;
    
    /** Size of the memory region. */
    Size size;

    /** Page protection flags. */
    u16 flags;
    
    /** Memory contents. */
    u8 *data;
}
MemoryRegion;

/** Entry point of a program. */
typedef Address EntryPoint;

/** Forward declaration. */
class ExecutableFormat;

/**
 * Confirms if we understand the given format.
 * @return true on success and false on failure.
 */
typedef ExecutableFormat * FormatDetector(const char *path);

/**
 * Abstraction class of various executable formats.
 */
class ExecutableFormat
{
    public:

	/**
	 * Class constructor.
	 * @param path Filesystem path to the executable.
	 */
	ExecutableFormat(const char *path);

	/**
	 * Class destructor.
	 */
	virtual ~ExecutableFormat();

	/**
	 * Retrieve path to the executable.
	 * @return Path on filesystem to the executable.
	 */
	const char * getPath()
	{
	    return path;
	}
    
	/**
	 * Memory regions a program needs at runtime.
	 * @param regions Memory regions to fill.
	 * @param max Maximum number of memory regions.
	 * @return Number of memory regions or an error code on error.
	 */
	virtual int regions(MemoryRegion *regions, Size max) = 0;

	/**
	 * Lookup the program entry point.
	 * @return Program entry point.
	 */
	virtual Address entry() = 0;

	/**
	 * Find a ExecFormat which can handle the given format.
	 * @param path Path to the file to read.
	 * @return A pointer to an ExecutableFormat on success
	 *         and NULL if not found.
	 */
	static ExecutableFormat * find(const char *path);

    private:
    
	/** Path to the executable. */
	const char *path;
};

/**
 * @}
 */

#endif /* __ASSEMBLER__ */
#endif /* __LIBEXEC_EXECUTABLEFORMAT_H */
