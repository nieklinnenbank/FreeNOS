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

#include <FreeNOS/System.h>
#include <ExecutableFormat.h>
#include <Types.h>
#include <Runtime.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "unistd.h"

int forkexec(const char *path, const char *argv[])
{
    int fd;
    struct stat st;
    u8 *image;

    // Find program image
    if (stat(path, &st) != 0)
        return -1;

    // Open program image
    if ((fd = open(path, O_RDONLY)) < 0)
        return -1;

    // Read the program image
    image = new u8[st.st_size];
    if (read(fd, image, st.st_size) != st.st_size)
    {
        delete image;
        close(fd);
        return -1;
    }
    close(fd);

    // Spawn the new program
    int ret = spawn((Address)image, st.st_size, argv);

    // Cleanup resources
    delete image;
    return ret;
}
