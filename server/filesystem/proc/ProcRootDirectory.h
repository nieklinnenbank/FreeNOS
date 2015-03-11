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

#ifndef __FILESYSTEM_PROCROOTDIRECTORY_H
#define __FILESYSTEM_PROCROOTDIRECTORY_H

#include <Directory.h>
#include <Types.h>
#include <Error.h>
#include "ProcFileSystem.h"
#include "IOBuffer.h"

/** @see ProcFileSystem */
class ProcFileSystem;

/** 
 * @defgroup procfs procfs (Process Filesystem) 
 * @{ 
 */

/**
 * Lists running processes as directory entries.
 */
class ProcRootDirectory : public Directory
{
    public:

	/**
	 * Constructor function.
	 * @param proc ProcFileSystem instance.
	 */
	ProcRootDirectory(ProcFileSystem *p);

        /** 
         * @brief Read bytes from the file. 
	 *
         * @param buffer Output buffer. 
         * @param size Number of bytes to read, at maximum. 
         * @param offset Offset inside the file to start reading. 
         * @return Number of bytes read on success, Error on failure. 
	 *
	 * @see IOBuffer
         */
        Error read(IOBuffer *buffer, Size size, Size offset);

    private:

	/** ProcFS instance for which we are spawned. */
	ProcFileSystem *proc;    
};

/**
 * @}
 */

#endif /* __FILESYSTEM_PROCROOTDIRECTORY_H */
