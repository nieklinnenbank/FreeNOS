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

#include <List.h>
#include <Types.h>

/**
 * Abstracts a memory region read from a format.
 */
typedef struct MemoryRegion
{
    /** Beginning and end of the region. */
    Address from, to;

    /** Indicates at which offset in the file to read the contents. */
    u64 offset;
}
MemoryRegion;

/** Entry point of a program. */
typedef Address EntryPoint;

/**
 * Abstraction class of various executable formats.
 */
class ExecutableFormat
{
    public:

	/**
	 * Class constructor.
	 */
	ExecutableFormat();

	/**
	 * Class destructor.
	 */
	virtual ~ExecutableFormat();

	/**
	 * Confirms if we understand the given format.
	 * @param path Path to the file to read.
	 * @return true on success and false on failure.
	 */
	virtual bool detect(const char *path) = 0;
    
	/**
	 * Memory regions a program needs at runtime.
	 * @param cb Is executed to read input data.
	 * @param regions Memory regions to fill.
	 * @param max Maximum number of memory regions.
	 * @return Number of memory regions or an error code on error.
	 */
	virtual int regions(const char *path, MemoryRegion *regions,
			    Size max) = 0;

	/**
	 * Lookup the program entry point.
	 * @param path File to read from.
	 * @param buf Entry point is written here.
	 * @return Zero on success and an error code on error.
	 */
	virtual int entry(const char *path, Address *buf) = 0;

	/**
	 * Find a ExecFormat which can handle the given format.
	 * @param path Path to the file to read.
	 * @return A pointer to an ExecutableFormat on success
	 *         and NULL if not found.
	 */
	static ExecutableFormat * find(const char *path);

    protected:

	/** List of known executable formats. */
	static List<ExecutableFormat> formats;
};

#endif /* __LIBEXEC_EXECUTABLEFORMAT_H */
