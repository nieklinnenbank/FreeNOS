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

#include <Init.h>
#include "POSIXSupport.h"
#include <string.h>
#include "unistd.h"

Shared<FileSystemMount> mounts;
Shared<FileDescriptor> files;
Shared<UserProcess> procs;

void setup()
{
    char key[256];

    if (getpid() != MEMSRV_PID)
    {
	/* Load the mounts and process table. */
        mounts.load(FILE_SYSTEM_MOUNT_KEY, MAX_MOUNTS);
        procs.load(USER_PROCESS_KEY, MAX_PROCS);
    
        /* Format FileDescriptor key. */
        snprintf(key, sizeof(key), "%s%u", FILE_DESCRIPTOR_KEY, getpid());
    
        /* Then load the FileDescriptor table. */
        files.load(key, FILE_DESCRIPTOR_MAX);
    }
}

ProcessID findMount(const char *path)
{
    FileSystemMount *m = ZERO;
    Size length = 0, len;
    char tmp[PATHLEN];

    /* Is the path relative? */
    if (path[0] != '/')						
    {
	getcwd(tmp, sizeof(tmp));
	snprintf(tmp, sizeof(tmp), "%s/%s", tmp, path);
    }
    else
	strlcpy(tmp, path, PATHLEN);
	
    /* Find the longest match. */
    for (Size i = 0; i < MAX_MOUNTS; i++)
    {
	if (mounts[i]->path[0])
	{
	    len = strlen(mounts[i]->path);
    
	    /*
	     * Only choose this mount, if it matches,
	     * and is longer than the last match.
	     */
	    if (strncmp(tmp, mounts[i]->path, len) == 0 && len > length)
	    {
		length = len;
		m = mounts[i];
	    }
	}
    }
    /* All done. */
    return m ? m->procID : ZERO;
}

ProcessID findMount(int fildes)
{
    return files[fildes] ? files[fildes]->mount : ZERO;
}

INITFUNC(setup, LIBCRT_DEFAULT)
