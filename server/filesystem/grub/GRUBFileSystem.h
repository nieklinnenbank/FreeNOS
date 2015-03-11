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

#ifndef __FILESYSTEM_GRUBFILESYSTEM_H
#define __FILESYSTEM_GRUBFILESYSTEM_H

#include <File.h>
#include <FileSystem.h>
#include <FileSystemMessage.h>
#include <Types.h>
#include <Error.h>

/** 
 * @defgroup grubfs grubfs (GRUB Filesystem) 
 * @{ 
 */

/**
 * @brief GRUB filesystem implementation (grubfs).
 * Maps information and modules retrieved from GRUB into a pseudo filesystem.
 */
class GRUBFileSystem : public FileSystem
{
    public:
    
	/**
	 * Class constructor function.
	 * @param path Path to which we are mounted.
	 */
	GRUBFileSystem(const char *path);
};

/**
 * @}
 */

#endif /* __FILESYSTEM_GRUBFILESYSTEM_H */
