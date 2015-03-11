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

#ifndef __FILESYSTEM_FILE_DESCRIPTOR_H
#define __FILESYSTEM_FILE_DESCRIPTOR_H

#include "FileSystemMount.h"
#include <Array.h>
#include <Shared.h>
#include <Types.h>
#include <Macros.h>

/** File descriptor table key prefix. */
#define FILE_DESCRIPTOR_KEY "FileDescriptor/"

/** Maximum number of open file descriptors. */
#define FILE_DESCRIPTOR_MAX 1024

/**
 * Abstracts a file which is opened by a user process.
 */
typedef struct FileDescriptor
{
    /**
     * Constructor function.
     * @param mnt ProcessID of the filesystem server.
     * @param ident Unique identifier.
     */
    FileDescriptor(ProcessID mnt, Address ident)
	: mount(mnt), identifier(ident), position(ZERO)
    {
    }

    /** Filesystem server on which this file was opened. */
    ProcessID mount;

    /** Unique identifier, used by the underlying filesystem. */
    Address identifier;

    /** Current position indicator. */
    Size position;
}
FileDescriptor;

/**  
 * Retrieve a pointer to the FileDescriptor table.  
 * @param files Per-process FileDescriptor table.
 * @param procID Process Identity number.  
 * @return Pointer to the (shared) FileDescriptor table.  
 */
inline Shared<FileDescriptor> * getFileDescriptors(Array<Shared<FileDescriptor> > *files,
					    ProcessID procID)
{
    Shared<FileDescriptor> *fds;
    char key[128];

    /* Do we have it loaded already? */
    if ((fds = files->get(procID)))
    {
        return fds;
    }
    /* Construct key. */
    snprintf(key, sizeof(key), "%s%u", FILE_DESCRIPTOR_KEY, procID);

    /* Load the object. */
    fds = new Shared<FileDescriptor>(key, FILE_DESCRIPTOR_MAX);

    /* Store in our Array. */
    files->insert(procID, fds);

    /* Done. */
    return fds;
}

/**  
 * Retrieve a pointer to the FileDescriptor of the given index.  
 * @param procID Process Identity.  
 * @param index Index number of the FileDescriptor in the table.  
 */
inline FileDescriptor * getFileDescriptor(Array<Shared<FileDescriptor> > *files,
				   ProcessID procID, int index)
{
    return getFileDescriptors(files, procID)->get(index);
}

#endif /* __FILESYSTEM_FILE_DESCRIPTOR_H */
