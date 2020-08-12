/*
 * Copyright (C) 2015 Niek Linnenbank
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

#include <FreeNOS/User.h>
#include <Types.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "unistd.h"

int forkexec(const char *path, const char *argv[])
{
    int fd, ret = 0;
    struct stat st;
    Memory::Range range;

    // Find program image
    if (stat(path, &st) != 0)
        return -1;

    // Open program image
    if ((fd = open(path, O_RDONLY)) < 0)
        return -1;

    // Map memory buffer for the program image
    range.virt   = ZERO;
    range.phys   = ZERO;
    range.size   = st.st_size;
    range.access = Memory::User|Memory::Readable|Memory::Writable;

    // Create mapping
    if (VMCtl(SELF, MapContiguous, &range) != API::Success)
    {
        errno = EFAULT;
        return -1;
    }

    // Read the program image
    ret = read(fd, (void *)range.virt, st.st_size);

    // Close file handle
    close(fd);

    // Spawn the new program
    if (ret == st.st_size)
    {
        ret = spawn(range.virt, st.st_size, argv);
    }

    // Cleanup program buffer
    if (VMCtl(SELF, Release, &range) != API::Success)
    {
        errno = EFAULT;
        return -1;
    }

    return ret;
}
