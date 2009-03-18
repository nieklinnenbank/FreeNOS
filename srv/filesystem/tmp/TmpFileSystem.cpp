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

    root = new FileCache(&slash, new Directory, ZERO);
}

void TmpFileSystem::createFileHandler(FileSystemMessage *msg,
				      FileSystemMessage *reply)
{
    char path[PATHLEN];
    
    /* Copy the path first. */
    if (VMCopy(msg->from, Read, (Address) path,
                                (Address) msg->buffer, PATHLEN) <= 0)
    {
        reply->result = EACCESS;
        return;
    }
    /* Create the appropriate file type. */
    switch (msg->filetype)
    {
	case S_IFREG:
	    insertFileCache(new File, "%s", path);
	    break;
	
	case S_IFDIR:
	    insertFileCache(new Directory, "%s", path);
	    break;
	
	case S_IFCHR:
	    insertFileCache(new Device(msg->deviceID), "%s", path);
	    break;
	
	default:
	    reply->result = EINVALID;
	    return;
    }
    reply->result = ESUCCESS;
}
