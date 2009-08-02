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

#ifndef __LIBPOSIX_SUPPORT_H
#define __LIBPOSIX_SUPPORT_H

#include <FileSystemMount.h>
#include <FileDescriptor.h>
#include <UserProcess.h>
#include <Types.h>
#include <Macros.h>
#include <Array.h>
#include <Shared.h>

/**
 * @defgroup libposix libposix (POSIX.1-2008) 
 * @{ 
 */

/**
 * Retrieve the ProcessID of the FileSystemMount for the given path.
 * @param path Path to lookup.
 * @return ProcessID of the FileSystemMount on success and ZERO otherwise.
 */
ProcessID findMount(const char *path);

/**
 * Lookup the ProcessID of a FileSystemMount by a filedescriptor number.
 * @param fildes FileDescriptor number.
 * @return ProcessID of the FileSystemMount on success and ZERO otherwise.
 */
ProcessID findMount(int fildes);

/** Mounted filesystems. */
extern Shared<FileSystemMount> mounts;

/** User process table. */
extern Shared<UserProcess> procs;

/** File descriptors. */
extern Shared<FileDescriptor> files;

/**
 * @}
 */

#endif /* __LIBPOSIX_FILESYSTEM_H */
