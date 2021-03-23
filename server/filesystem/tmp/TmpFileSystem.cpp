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
#include <File.h>
#include <PseudoFile.h>
#include <Directory.h>
#include "TmpFileSystem.h"

TmpFileSystem::TmpFileSystem(const char *path)
    : FileSystemServer(new Directory(1), path)
{
}

File * TmpFileSystem::createFile(const FileSystem::FileType type)
{
    // Create the appropriate file type
    switch (type)
    {
        case FileSystem::RegularFile: {
            PseudoFile *file = new PseudoFile(getNextInode());
            assert(file != NULL);
            return file;
        }

        case FileSystem::DirectoryFile: {
            Directory *dir = new Directory(getNextInode());
            assert(dir != NULL);
            return dir;
        }

        default:
            return ZERO;
    }
}
