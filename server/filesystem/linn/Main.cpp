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
#include <KernelLog.h>
#include <FileStorage.h>
#include <BootImageStorage.h>
#include <BootSymbolStorage.h>
#include "LinnFileSystem.h"

int main(int argc, char **argv)
{
    KernelLog log;
    Storage *storage = ZERO;
    const char *path = "/";
    SystemInformation info;

    // Only run on core0
    if (info.coreId != 0)
        return 0;

    // Mount the given file, or try to use the BootImage embedded rootfs
    if (argc > 3)
    {
        const String offsetStr(argv[2], false);
        const Size offset = offsetStr.toLong();
        NOTICE("file storage: " << argv[1] << " at offset " << offset);
        storage = new FileStorage(argv[1], offset);
        assert(storage != NULL);
        path = argv[3];
    }
    else
    {
        // Allocate BootImage
        BootImageStorage *bm = new BootImageStorage();
        assert(bm != NULL);

        // Load BootImage
        const FileSystem::Result imageResult = bm->initialize();
        if (imageResult != FileSystem::Success)
        {
            FATAL("unable to load BootImage: result = " << (int) imageResult);
        }

        // Allocate BootSymbol
        BootSymbolStorage *bs = new BootSymbolStorage(*bm, LINNFS_ROOTFS_FILE);
        assert(bs != NULL);

        // Load BootSymbol
        const FileSystem::Result symbolResult = bs->initialize();
        if (symbolResult != FileSystem::Success)
        {
            FATAL("unable to load BootSymbol '" << LINNFS_ROOTFS_FILE <<
                  "': result = " << (int) symbolResult);
        }

        storage = bs;
        NOTICE("boot image: " << LINNFS_ROOTFS_FILE);
    }

    // Mount, then start serving requests.
    if (storage)
    {
        LinnFileSystem server(path, storage);
        server.mount();
        return server.run();
    }

    ERROR("no usable storage found");
    return 1;
}
