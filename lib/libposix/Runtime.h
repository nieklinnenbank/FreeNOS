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

#ifndef __LIBPOSIX_RUNTIME_H
#define __LIBPOSIX_RUNTIME_H

#include <Macros.h>
#include <Types.h>
#include <Vector.h>
#include <String.h>
#include <ChannelClient.h>
#include "FileSystemMount.h"
#include "FileDescriptor.h"

/**
 * @defgroup libposix libposix (POSIX.1-2008) 
 * @{ 
 */

/**
 * C(++) program entry point.
 * @param argc Argument count.
 * @param argv Argument values.
 * @return Exit status.
 */
int main(int argc, char **argv);

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

void refreshMounts(const char *path);

/**
 * Get FileDescriptors table.
 */
Vector<FileDescriptor> * getFiles();

/**
 * Get mounts table.
 */
FileSystemMount * getMounts();

/**
 * Get current directory String.
 */
String * getCurrentDirectory();

/**
 * @}
 */

#endif /* __LIBPOSIX_RUNTIME_H */
