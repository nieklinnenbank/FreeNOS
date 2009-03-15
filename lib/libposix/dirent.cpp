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

#include <api/IPCMessage.h>
#include <Allocator.h>
#include <Config.h>
#include <FileSystemMessage.h>
#include <errno.h>
#include "dirent.h"
#include "fcntl.h"
#include "unistd.h"

DIR * opendir(const char *dirname)
{
    DIR *dir;
    int fd;
    
    /* Try to open the directory. */
    if ((fd = open(dirname, ZERO)) < 0)
    {
	return (ZERO);
    }
    /* Allocate DIR object. */
    dir = new DIR;
    dir->fd        = fd;
    dir->buffer    = new Dirent[8];
    dir->current   = 8;
    dir->count     = 8;
    dir->countRead = 0;
    dir->eof       = false;
    
    /* Set errno. */
    errno = ESUCCESS;
    
    /* Success. */
    return dir;
}

struct dirent * readdir(DIR *dirp)
{
    FileSystemMessage msg;
    Size num; 

    /* Do we need to read more dirents? */
    if (dirp->current >= dirp->count && !dirp->eof)
    {
	/* Read more entries. */
	if (!(num = read(dirp->fd, dirp->buffer, sizeof(Dirent) * 8)))
	{
	    dirp->eof = true;
	}
	else
	{
	    dirp->current   = 0;
	    dirp->countRead = num / sizeof(Dirent);
	}
    }
    /* Retrieve next dirent. */
    if (dirp->current < dirp->countRead)
	return &dirp->buffer[dirp->current++];
    else
	return (struct dirent *) ZERO;
}

int closedir(DIR *dirp)
{
    /* Close file handle. */
    close(dirp->fd);

    /* Free buffers. */
    delete dirp->buffer;
    delete dirp;
    
    /* Success. */
    return 0;
}
