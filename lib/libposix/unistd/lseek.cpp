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

#include <FileDescriptor.h>
#include "errno.h"
#include "unistd.h"

off_t lseek(int fildes, off_t offset, int whence)
{
    // Do we have this file descriptor?
    FileDescriptor::Entry *fd = FileDescriptor::instance()->getEntry(fildes);
    if (!fd || !fd->open)
    {
        errno = ENOENT;
        return -1;
    }

    // Update the file position pointer
    fd->position = offset;

    // Done
    return 0;
}
