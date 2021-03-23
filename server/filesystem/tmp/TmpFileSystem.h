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

#ifndef __FILESYSTEM_TMPFILESYSTEM_H
#define __FILESYSTEM_TMPFILESYSTEM_H

#include <FileSystem.h>
#include <FileSystemServer.h>
#include <Types.h>

/**
 * @addtogroup server
 * @{
 *
 * @addtogroup tmpfs
 * @{
 */

/**
 * Temporary filesystem (TmpFS). Maps files into virtual memory.
 */
class TmpFileSystem : public FileSystemServer
{
  public:

    /**
     * Class constructor function.
     *
     * @param path Path to which we are mounted.
     */
    TmpFileSystem(const char *path);

    /**
     * @brief Creates a new TmpFile.
     *
     * @param type Describes the file type to create.
     *
     * @return Pointer to a new File on success, or ZERO on failure.
     *
     * @see File
     */
    virtual File * createFile(const FileSystem::FileType type);
};

/**
 * @}
 * @}
 */

#endif /* __FILESYSTEM_TMPFILESYSTEM_H */
