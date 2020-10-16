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
#include <Lz4Decompressor.h>
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

    // Find program image
    if (stat(path, &st) != 0)
        return -1;

    // Open program image
    if ((fd = open(path, O_RDONLY)) < 0)
        return -1;

    // Map memory buffer for the compressed program image
    Memory::Range compressed;
    compressed.virt   = ZERO;
    compressed.phys   = ZERO;
    compressed.size   = st.st_size;
    compressed.access = Memory::User|Memory::Readable|Memory::Writable;

    // Create mapping
    if (VMCtl(SELF, MapContiguous, &compressed) != API::Success)
    {
        errno = EFAULT;
        return -1;
    }

    // Read the program image
    ret = read(fd, (void *) compressed.virt, st.st_size);

    // Close file handle
    close(fd);

    if (ret != st.st_size)
    {
        VMCtl(SELF, Release, &compressed);
        errno = EIO;
        return -1;
    }

    // Initialize decompressor
    Lz4Decompressor lz4((const void *)compressed.virt, st.st_size);
    const Lz4Decompressor::Result result = lz4.initialize();
    if (result != Lz4Decompressor::Success)
    {
        VMCtl(SELF, Release, &compressed);
        errno = EFAULT;
        return -1;
    }

    // Map memory buffer for the uncompressed program image
    Memory::Range uncompressed;
    uncompressed.virt   = ZERO;
    uncompressed.phys   = ZERO;
    uncompressed.size   = lz4.getUncompressedSize();
    uncompressed.access = Memory::User|Memory::Readable|Memory::Writable;

    // Create mapping
    if (VMCtl(SELF, MapContiguous, &uncompressed) != API::Success)
    {
        VMCtl(SELF, Release, &compressed);
        errno = EFAULT;
        return -1;
    }

    // Decompress entire file
    const Lz4Decompressor::Result readResult = lz4.read((void *)uncompressed.virt, lz4.getUncompressedSize());
    if (readResult != Lz4Decompressor::Success)
    {
        VMCtl(SELF, Release, &compressed);
        VMCtl(SELF, Release, &uncompressed);
        errno = EFAULT;
        return -1;
    }

    // Cleanup compressed program buffer
    if (VMCtl(SELF, Release, &compressed) != API::Success)
    {
        VMCtl(SELF, Release, &uncompressed);
        errno = EFAULT;
        return -1;
    }

    // Spawn the new program
    ret = spawn(uncompressed.virt, lz4.getUncompressedSize(), argv);

    // Cleanup uncompressed program buffer
    if (VMCtl(SELF, Release, &uncompressed) != API::Success)
    {
        errno = EFAULT;
        return -1;
    }

    return ret;
}
