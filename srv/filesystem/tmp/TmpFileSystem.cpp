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
#include <Device.h>
#include "TmpFileSystem.h"
#include "TmpFile.h"

TmpFileSystem::TmpFileSystem(const char *path)
    : FileSystem(path)
{
    FileSystemPath slash("/");
    Directory *rdir = new Directory;

    root = new FileCache(&slash, rdir, ZERO);
    rdir->insertEntry(".", DT_DIR);
    rdir->insertEntry("..", DT_DIR);
    insertFileCache(rdir, ".");
    insertFileCache(rdir, "..");
}

Error TmpFileSystem::createFile(FileSystemMessage *msg,
				FileSystemPath *path)
{
    Directory *pdir;

    /* Create the appropriate file type. */
    switch (msg->filetype)
    {
	case S_IFREG:
	    insertFileCache(new File, "%s", **path->full());
	    break;
	
	case S_IFDIR:
	    insertFileCache(new Directory, "%s", **path->full());
	    break;
	
	case S_IFCHR:
	    insertFileCache(new Device(msg->deviceID), "%s", **path->full());
	    break;
	
	default:
	    return EINVALID;
    }
    /* Add directory entry. */
    if (path->parent())
    {
	pdir = (Directory *) findFileCache(**path->parent())->file;
    }
    else
	pdir = (Directory *) root->file;
	
    pdir->insertEntry(**path->full(), msg->filetype);
    
    /* All done. */
    return ESUCCESS;
}
