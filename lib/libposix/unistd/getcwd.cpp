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

#include <Assert.h>
#include <FileSystemClient.h>
#include <string.h>
#include <errno.h>
#include "unistd.h"

char *getcwd(char *buf, size_t size)
{
    const FileSystemClient filesystem;
    const String *currentDirectory = filesystem.getCurrentDirectory();

    // Copy our current working directory
    assert(currentDirectory != NULL);
    memcpy(buf, **currentDirectory, size);

    // Set errno
    errno = ESUCCESS;

    // Done
    return buf;
}
