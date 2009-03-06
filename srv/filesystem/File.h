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

#ifndef __FILESYSTEM_FILE_H
#define __FILESYSTEM_FILE_H

#include <Types.h>
#include <Error.h>

/**
 * All possible filetypes.
 */
typedef enum FileType
{
    RegularFile         = 0,
    DirectoryFile       = 1,
    BlockDeviceFile     = 2,
    CharacterDeviceFile = 3,
    SymlinkFile         = 4,
    FIFOFile            = 5,
};

/**
 * Abstracts a file which is opened by a process.
 */
class File
{
    public:
    
	/**
	 * Constructor function.
	 * @param t Type of file.
	 */
	File(FileType t = RegularFile) : type(t), size(ZERO)
	{
	}

	/**
	 * Destructor function.
	 */
	virtual ~File()
	{
	}
    
	/**
	 * Read bytes from the file.
	 * @param buffer Output buffer.
	 * @param size Maximum size to read.
	 * @param offset Offset in the file to read.
	 * @return Number of bytes read on success, Error on failure.
	 */
	virtual Error read(u8 *buffer, Size size, Size offset)
	{
	    return ENOSUPPORT;
	}

	/**
	 * Write bytes to the file.
	 * @param buffer Input buffer.
	 * @param size Maximum size to write.
	 * @param offset Offset in the file to write.
	 * @return Number of bytes written on success, Error on failure.
	 */
	virtual Error write(u8 *buffer, Size size, Size offset)
	{
	    return ENOSUPPORT;
	}

    private:

	/** File of this file. */
	FileType type;
	
	/** Size of the file, in bytes. */
	Size size;
};

#endif /* __FILESYSTEM_FILE_H */
