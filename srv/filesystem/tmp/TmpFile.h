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

#ifndef __FILESYSTEM_TMPFILE_H
#define __FILESYSTEM_TMPFILE_H

#include <File.h>
#include <Directory.h>
#include <Types.h>
#include <Error.h>

/**
 * Keeps file contents in virtual memory instead of persistent storage.
 */
class TmpFile : public File
{
    public:

	/**
	 * Constructor function.
	 */
	TmpFile();

	/**
	 * Destructor function, which releases the buffer.
	 */
	~TmpFile();

	/**
	 * Reads out the buffer.
	 * @param buffer Output buffer.
	 * @param size Maximum number of bytes to write.
	 * @param offset Offset to read.
	 * @return Number of bytes read, or Error number.
	 */
	Error read(u8 *buffer, Size size, Size offset);

        /** 
         * Write bytes to the file.
         * @param buffer Input buffer.
         * @param size Maximum size to write.
         * @param offset Offset in the file to write.
         * @return Number of bytes written on success, Error on failure.
         */
	Error write(u8 *buffer, Size size, Size offset);

    private:
    
	/** Contains the file contents. */
	u8 *buffer;
	
	/** Size of the buffer. */
	Size size;
};

#endif /* __FILESYSTEM_TMPFILE_H */
