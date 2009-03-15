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

#ifndef __FILESYSTEM_PROCFILE_H
#define __FILESYSTEM_PROCFILE_H

#include <File.h>
#include <Directory.h>
#include <Types.h>
#include <Error.h>

/**
 * Maps running processes into the filesystem.
 */
class ProcFile : public File
{
    public:

	/**
	 * Constructor function.
	 * @param str Input string.
	 */
	ProcFile(char *str);

	/**
	 * Destructor function, which releases the buffer.
	 */
	~ProcFile();

	/**
	 * Reads out the buffer.
	 * @param buffer Output buffer.
	 * @param size Maximum number of bytes to write.
	 * @param offset Offset to read.
	 * @return Number of bytes read, or Error number.
	 */
	Error read(u8 *buffer, Size size, Size offset);

    private:
    
	/** Buffer from which we read. */
	char *buffer;
	
	/** Size of the buffer. */
	Size size;
};

#endif /* __FILESYSTEM_PROCFILE_H */
