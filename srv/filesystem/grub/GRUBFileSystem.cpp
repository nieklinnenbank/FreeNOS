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

#include <API/SystemInfo.h>
#include <Directory.h>
#include <PseudoFile.h>
#include "GRUBFileSystem.h"
#include "GRUBModule.h"
#include <libgen.h>

GRUBFileSystem::GRUBFileSystem(const char *path)
    : FileSystem(path)
{
    Directory *slash = new Directory;
    Directory *mods  = new Directory;
    SystemInformation info;

    /* Set root directory. */
    setRoot(slash);

    /* Dot and DotDot. */
    insertFileCache(slash, ".");
    insertFileCache(slash, "..");
    insertFileCache(mods,  "modules");

    /* Insert directory entries. */
    slash->insert(DirectoryFile, ".");
    slash->insert(DirectoryFile, "..");
    slash->insert(DirectoryFile, "modules");
    slash->insert(RegularFile, "cmdline");
    slash->insert(RegularFile, "memory_size");
    slash->insert(RegularFile, "memory_avail");

    /* Kernel Command-line. */
    insertFileCache(new PseudoFile("%s", info.cmdline), "cmdline");
    
    /* Memory information. */
    insertFileCache(new PseudoFile("%x", info.memorySize),  "memory_size");
    insertFileCache(new PseudoFile("%x", info.memoryAvail), "memory_avail");
    
    /* Boot Modules. */
    for (Size i = 0; i < info.moduleCount; i++)
    {
	insertFileCache(new GRUBModule(info.modules[i].string),
			"modules/%s",   basename(info.modules[i].string));
	mods->insert(RegularFile, "%s", basename(info.modules[i].string));
    }
}
