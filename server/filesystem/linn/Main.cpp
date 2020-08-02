/*
 * Copyright (C) 2020 Niek Linnenbank
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

#include <Types.h>
#include <Assert.h>
#include <FileStorage.h>
#include <BootImageStorage.h>
#include <stdlib.h>
#include "LinnFileSystem.h"

int main(int argc, char **argv)
{
    Storage *storage = ZERO;
    const char *path = "/";
    SystemInformation info;

    // Only run on core0
    if (info.coreId != 0)
        return EXIT_SUCCESS;

    // Mount the given file, or try to use the BootImage embedded rootfs
    if (argc > 3)
    {
        NOTICE("file storage: " << argv[1] << " at offset " << atoi(argv[2]));
        storage = new FileStorage(argv[1], atoi(argv[2]));
        assert(storage != NULL);
        path = argv[3];
    }
    else
    {
        BootImageStorage *bm = new BootImageStorage(LINNFS_ROOTFS_FILE);
        assert(bm != NULL);
        if (bm->load())
        {
            NOTICE("boot image: " << LINNFS_ROOTFS_FILE);
            storage = bm;
        }
        else
        {
            FATAL("unable to load: " << LINNFS_ROOTFS_FILE);
        }
    }

    // Mount, then start serving requests.
    if (storage)
    {
        LinnFileSystem server(path, storage);
        server.mount();
        return server.run();
    }

    ERROR("no usable storage found");
    return EXIT_FAILURE;
}
