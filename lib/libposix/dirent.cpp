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

#include <API/IPCMessage.h>
#include <Allocator.h>
#include <Config.h>
#include <FileSystemMessage.h>
#include <Directory.h>
#include <errno.h>
#include "dirent.h"
#include "fcntl.h"
#include "unistd.h"
#include "sys/stat.h"

DIR * opendir(const char *dirname)
{
    Dirent *dirent;
    DIR *dir;
    int fd;
    struct stat st;
    Error e;
    
    /* First stat the directory. */
    if (stat(dirname, &st) < 0)
    {
	return (ZERO);
    }
    /* Try to open the directory. */
    if ((fd = open(dirname, ZERO)) < 0)
    {
	return (ZERO);
    }
    /* Allocate Dirents. */
    dirent = new Dirent[st.st_size / sizeof(Dirent)];
    
    /* Allocate DIR object. */
    dir = new DIR;
    dir->fd        = fd;
    dir->buffer    = new struct dirent[st.st_size / sizeof(Dirent)];
    dir->current   = 0;
    dir->count     = 0;
    dir->eof       = false;
    
    /* Read them all. */
    if ((e = read(fd, dirent, sizeof(Dirent) * st.st_size)) < 0)
    {
	e = errno;
	closedir(dir);
	errno = e;
	return (ZERO);
    }
    /* Fill in the dirent structs. */
    for (Size i = 0; i < st.st_size / sizeof(Dirent); i++)
    {
	u8 types[] =
	{
	    DT_REG,
	    DT_DIR,
	    DT_BLK,
	    DT_CHR,
	    DT_LNK,
	    DT_FIFO,
	    DT_SOCK,
	};
	strlcpy(dir->buffer[i].d_name, dirent[i].name, DIRLEN);
	dir->buffer[i].d_type = types[dirent[i].type];
    }
    dir->count = e / sizeof(Dirent);
    delete dirent;

    /* Set errno. */
    errno = ESUCCESS;
    
    /* Success. */
    return dir;
}

struct dirent * readdir(DIR *dirp)
{
    if (dirp->current < dirp->count)
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
