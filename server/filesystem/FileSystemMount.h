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

#ifndef __FILESYSTEM_FILE_SYSTEM_MOUNT_H
#define __FILESYSTEM_FILE_SYSTEM_MOUNT_H

#include "FileSystemPath.h"
#include <Types.h>

/** Maximum number of mounted filesystems. */
#define MAX_MOUNTS 16

/** Shared mapping key for the mounts table. */
#define FILE_SYSTEM_MOUNT_KEY "FileSystemMount"

/**
 * Represents a mounted filesystem.
 */
typedef struct FileSystemMount
{
    /** Path of the mount. */
    char path[PATHLEN];
    
    /** Server which is responsible for the mount. */
    ProcessID procID;
    
    /** Mount options. */
    ulong options;
}
FileSystemMount;

#endif /* __FILESYSTEM_FILE_SYSTEM_MOUNT_H */
