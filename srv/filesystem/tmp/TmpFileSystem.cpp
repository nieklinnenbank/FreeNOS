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

#include <File.h>
#include <Directory.h>
#include <Special.h>
#include "TmpFileSystem.h"
#include "TmpFile.h"

TmpFileSystem::TmpFileSystem(const char *path)
    : FileSystem(path)
{
    setRoot(new Directory);
}

File * TmpFileSystem::createFile(FileType type, DeviceID deviceID)
{
    /* Create the appropriate file type. */
    switch (type)
    {
	case RegularFile:
	    return new TmpFile;
	
	case DirectoryFile:
	    return new Directory;
	
	case CharacterDeviceFile:
	    return new Special(CharacterDeviceFile, deviceID);
	
	case BlockDeviceFile:
	    return new Special(BlockDeviceFile, deviceID);
	
	default:
	    return ZERO;
    }
}
