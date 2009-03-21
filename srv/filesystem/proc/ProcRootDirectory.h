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
	 * Reads out the buffer.
	 * @param msg Read request.
	 * @return Number of bytes read, or Error number.
	 */
	Error read(FileSystemMessage *msg);

    private:

	/** ProcFS instance for which we are spawned. */
	ProcFileSystem *proc;    
};

#endif /* __FILESYSTEM_PROCROOTDIRECTORY_H */
