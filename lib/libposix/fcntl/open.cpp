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

#include <FreeNOS/API.h>
#include <FileSystemMessage.h>
#include "Runtime.h"
#include "errno.h"
#include "fcntl.h"
#include "sys/stat.h"

int open(const char *path, int oflag, ...)
{
    FileSystemMessage msg;
    ProcessID mnt = findMount(path);

    // TODO: perhaps we need the 'Index' class now, in libstd.
    Vector<FileDescriptor> *fds = getFiles();
    FileStat st;
    
    // Fill message
    msg.action = StatFile;
    msg.path   = (char *) path;
    msg.stat   = &st;
    msg.type   = IPCType;

    // Ask the FileSystem for the file.
    if (mnt)
    {
        IPCMessage(mnt, API::SendReceive, &msg, sizeof(msg));
    
        // Set errno
        errno = msg.result;

        if (msg.result == ESUCCESS)
        {
            // Insert into file descriptor table
            for (Size i = 0; i < fds->size(); i++)
            {
                if (!(*fds)[i].open)
                {
                    (*fds)[i].open  = true;
                    strlcpy((*fds)[i].path, path, PATHLEN);

                    // Devices should be contacted directly, instead of the filesystem
                    // where we found the device file.
                    if (st.type == BlockDeviceFile || st.type == CharacterDeviceFile)
                    {
                        (*fds)[i].mount      = st.deviceID.major;
                        (*fds)[i].identifier = st.deviceID.minor;
                    }
                    else
                    {
                        (*fds)[i].mount = mnt;
                        (*fds)[i].identifier = 0;
                    }
                    (*fds)[i].position = 0;
                    return i;
                }
            }
            // Too many open files
            errno = ENFILE;
        }
    }
    else
        errno = ENOENT;

    return -1;
}
