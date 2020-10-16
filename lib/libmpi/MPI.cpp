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
#include <MemoryChannel.h>
#include <CoreClient.h>
#include <Lz4Decompressor.h>
#include <Index.h>
#include <String.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "MPIMessage.h"
#include "mpi.h"

#define MPI_PROG_CMDLEN 512
#define MEMBASE(id) (memChannelBase.phys + (coreCount * PAGESIZE * 2 * (id)))

Size coreCount = 0;
Index<MemoryChannel, MPI_MAX_CHANNELS> *readChannel  = 0;
Index<MemoryChannel, MPI_MAX_CHANNELS> *writeChannel = 0;

int MPI_Init(int *argc, char ***argv)
{
    SystemInformation info;
    const CoreClient coreClient;
    struct stat st;
    char *programName = (*argv)[0];
    char programPath[64];
    char cmd[MPI_PROG_CMDLEN];
    u8 *programBuffer;
    int fd;
    Memory::Range memChannelBase;

    // If we are master (node 0):
    if (info.coreId == 0)
    {
        // provide -n COUNT, --help and other stuff in here too.
        // to influence the launching of more MPI programs
        const Core::Result result = coreClient.getCoreCount(coreCount);
        if (result != Core::Success)
        {
            printf("%s: failed to retrieve core count from CoreServer: %d\n",
                    programName, (int) result);
            return MPI_ERR_BAD_FILE;
        }

        // Read our own ELF program to a buffer and pass it to CoreServer
        // for creating new programs on the remote core.
        if (strncmp(programName, "/bin/", 5) != 0)
            snprintf(programPath, sizeof(programPath), "/bin/%s", programName);
        else
            strlcpy(programPath, programName, sizeof(programPath));

        if (stat(programPath, &st) != 0)
        {
            printf("%s: failed to stat '%s': %s\n",
                    programName, programPath, strerror(errno));
            return MPI_ERR_BAD_FILE;
        }

        Memory::Range progRange;
        progRange.virt = 0;
        progRange.phys = 0;
        progRange.size = st.st_size;
        progRange.access = Memory::User | Memory::Readable | Memory::Writable;
        API::Result vmResult = VMCtl(SELF, MapContiguous, &progRange);
        if (vmResult != API::Success)
        {
            printf("%s: failed to allocate program buffer: result = %d\n", (int)vmResult);
            return MPI_ERR_NO_MEM;
        }

        programBuffer = (u8 *) progRange.virt;
        assert(programBuffer != NULL);

        // Read ELF program
        if ((fd = open(programPath, O_RDONLY)) == -1)
        {
            printf("%s: failed to open '%s': %s\n",
                    programName, programPath, strerror(errno));
            return MPI_ERR_BAD_FILE;
        }
        if (read(fd, programBuffer, st.st_size) != st.st_size)
        {
            printf("%s: failed to read '%s': %s\n",
                    programName, programPath, strerror(errno));
            return MPI_ERR_BAD_FILE;
        }
        if (close(fd) != 0)
        {
            printf("%s: failed to close '%s': %s\n",
                    programName, programPath, strerror(errno));
            return MPI_ERR_BAD_FILE;
        }

        // Initialize decompressor
        Lz4Decompressor lz4(programBuffer, st.st_size);
        Lz4Decompressor::Result lz4Result = lz4.initialize();
        if (lz4Result != Lz4Decompressor::Success)
        {
            printf("%s: failed to initialize LZ4 decompressor: result = %d\n",
                    programName, (int) lz4Result);
            return MPI_ERR_BAD_FILE;
        }

        // Allocate memory for decompressed program
        Memory::Range uncompProgRange;
        uncompProgRange.virt = 0;
        uncompProgRange.phys = 0;
        uncompProgRange.size = lz4.getUncompressedSize();
        uncompProgRange.access = Memory::User | Memory::Readable | Memory::Writable;
        vmResult = VMCtl(SELF, MapContiguous, &uncompProgRange);
        if (vmResult != API::Success)
        {
            printf("%s: failed to allocate program buffer: result = %d\n",
                   programName, (int)vmResult);
            return MPI_ERR_NO_MEM;
        }

        programBuffer = (u8 *) uncompProgRange.virt;
        assert(programBuffer != NULL);

        // Decompress entire file
        const Lz4Decompressor::Result readResult = lz4.read(programBuffer, lz4.getUncompressedSize());
        if (readResult != Lz4Decompressor::Success)
        {
            printf("failed to decompress program buffer: result = %d\n",
                    programName, (int) readResult);
            return MPI_ERR_NO_MEM;
        }

        // Allocate memory space on the local processor for the whole
        // UniChannel array, NxN communication with MPI.
        // Then pass the channel offset physical address as an argument -addr 0x.... to spawn()
        memChannelBase.size = (PAGESIZE * 2) * (coreCount * coreCount);
        memChannelBase.phys = 0;
        memChannelBase.virt = 0;
        memChannelBase.access = Memory::Readable | Memory::Writable | Memory::User;
        if (VMCtl(SELF, MapContiguous, &memChannelBase) != API::Success)
        {
            printf("%s: failed to allocate MemoryChannel\n",
                    programName);
            return MPI_ERR_NO_MEM;
        }
        printf("%s: MemoryChannel at physical address %x\n",
                programName, memChannelBase.phys);

        // Clear channel pages
        MemoryBlock::set((void *) memChannelBase.virt, 0, memChannelBase.size);

        // Format program command
        snprintf(cmd, MPI_PROG_CMDLEN, "%s -a %x -c %d",
                 programPath, memChannelBase.phys, coreCount);
        for (int j = 1; j < *argc; j++)
        {
            strcat(cmd, " ");
            strcat(cmd, (*argv)[j]);
        }

        // now create the slaves using coreservers.
        for (Size i = 1; i < coreCount; i++)
        {
            const Core::Result result = coreClient.createProcess(i, (const Address) programBuffer,
                                                                 lz4.getUncompressedSize(), cmd);
            if (result != Core::Success)
            {
                printf("%s: failed to create process on core%d: result = %d\n",
                        programName, i, (int) result);
                return MPI_ERR_SPAWN;
            }
        }
    }
    else
    {
        // If we are slave (node N):
        // read the -addr argument, and map the UniChannels into our address space.
        for (int i = 1; i < (*argc); i++)
        {
            if (!strcmp((*argv)[i], "--addr") ||
                !strcmp((*argv)[i], "-a"))
            {
                if ((*argc) < i+1)
                    return MPI_ERR_ARG;

                String s = (*argv)[i+1];
                memChannelBase.phys = s.toLong(Number::Hex);
                i++;
            }
            else if (!strcmp((*argv)[i], "--cores") ||
                     !strcmp((*argv)[i], "-c"))
            {
                if ((*argc) < i+1)
                    return MPI_ERR_ARG;
                coreCount = atoi((*argv)[i+1]);
                i++;
            }
            // Unknown MPI argument. Pass the rest to the user program.
            else
            {
                (*argc) -= (i-1);
                (*argv)[i-1] = (*argv)[0];
                (*argv) += (i-1);
                break;
            }
        }
    }

    // Create MemoryChannels
    assert(coreCount <= MPI_MAX_CHANNELS);
    readChannel  = new Index<MemoryChannel, MPI_MAX_CHANNELS>();
    assert(readChannel != NULL);
    writeChannel = new Index<MemoryChannel, MPI_MAX_CHANNELS>();
    assert(writeChannel != NULL);

    // Fill read channels
    for (Size i = 0; i < coreCount; i++)
    {
        MemoryChannel *ch = new MemoryChannel(Channel::Consumer, sizeof(MPIMessage));
        assert(ch != NULL);
        ch->setPhysical(MEMBASE(info.coreId) + (PAGESIZE * 2 * i),
                        MEMBASE(info.coreId) + (PAGESIZE * 2 * i) + PAGESIZE);
        readChannel->insertAt(i, ch);

        if (info.coreId == 0)
        {
            printf("%s: read: core%d: data=%x feedback=%x base%d=%x\n", (*argv)[0], i,
                    MEMBASE(info.coreId) + (PAGESIZE * 2 * i),
                    MEMBASE(info.coreId) + (PAGESIZE * 2 * i) + PAGESIZE,
                    i, MEMBASE(i));
        }
    }

    // Fill write channels
    for (Size i = 0; i < coreCount; i++)
    {
        MemoryChannel *ch = new MemoryChannel(Channel::Producer, sizeof(MPIMessage));
        assert(ch != NULL);
        ch->setPhysical(MEMBASE(i) + (PAGESIZE * 2 * info.coreId),
                        MEMBASE(i) + (PAGESIZE * 2 * info.coreId) + PAGESIZE);
        writeChannel->insertAt(i, ch);

        if (info.coreId == 0)
        {
            printf("%s: write: core%d: data=%x feedback=%x base%d=%x\n", (*argv)[0], i,
                    MEMBASE(i) + (PAGESIZE * 2 * info.coreId),
                    MEMBASE(i) + (PAGESIZE * 2 * info.coreId) + PAGESIZE,
                    i, MEMBASE(i));
        }
    }

    return MPI_SUCCESS;
}

int MPI_Finalize(void)
{
    return MPI_SUCCESS;
}

int MPI_Get_processor_name(char *name, int *resultlen)
{
    return MPI_SUCCESS;
}

int MPI_Get_version(int *version, int *subversion)
{
    *version = 3;
    *subversion = 1;
    return MPI_SUCCESS;
}

int MPI_Get_library_version(char *version, int *resultlen)
{
    return MPI_SUCCESS;
}
