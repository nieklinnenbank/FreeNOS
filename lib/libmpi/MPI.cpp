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
#include <MemoryBlock.h>
#include <BufferedFile.h>
#include <Log.h>
#include "MPIMessage.h"
#include "mpi.h"

#define MEMBASE(id) (memChannelBase.phys + (coreCount * PAGESIZE * 2 * (id)))

Size coreCount = 0;
Index<MemoryChannel, MPI_MAX_CHANNELS> *readChannel  = 0;
Index<MemoryChannel, MPI_MAX_CHANNELS> *writeChannel = 0;

int MPI_Init(int *argc, char ***argv)
{
    SystemInformation info;
    const CoreClient coreClient;
    char *programName = (*argv)[0];
    String programPath;
    String programCmd;
    u8 *programBuffer;
    Memory::Range memChannelBase;

    // If we are master (node 0):
    if (info.coreId == 0)
    {
        // Retrieve number of cores on the system
        const Core::Result result = coreClient.getCoreCount(coreCount);
        if (result != Core::Success)
        {
            ERROR("failed to retrieve core count from CoreServer: result = " << (int) result);
            return MPI_ERR_BAD_FILE;
        }

        // Read our own ELF program to a buffer and pass it to CoreServer
        // for creating new programs on the remote core.
        if (!MemoryBlock::compare(programName, "/bin/", 5))
        {
            programPath << "/bin/" << programName;
        }
        else
        {
            programPath << programName;
        }

        // Try to read the raw ELF program data (compressed)
        BufferedFile programFile(*programPath);
        const BufferedFile::Result readResult = programFile.read();
        if (readResult != BufferedFile::Success)
        {
            ERROR("failed to read program at path '" << *programPath << "': result = " << (int) readResult);
            return MPI_ERR_BAD_FILE;
        }

        // Initialize decompressor
        Lz4Decompressor lz4(programFile.buffer(), programFile.size());
        Lz4Decompressor::Result lz4Result = lz4.initialize();
        if (lz4Result != Lz4Decompressor::Success)
        {
            ERROR("failed to initialize LZ4 decompressor: result = " << (int) lz4Result);
            return MPI_ERR_BAD_FILE;
        }

        // Allocate memory for decompressed program
        Memory::Range uncompProgRange;
        uncompProgRange.virt = 0;
        uncompProgRange.phys = 0;
        uncompProgRange.size = lz4.getUncompressedSize();
        uncompProgRange.access = Memory::User | Memory::Readable | Memory::Writable;
        API::Result vmResult = VMCtl(SELF, MapContiguous, &uncompProgRange);
        if (vmResult != API::Success)
        {
            ERROR("failed to allocate program buffer: result = " << (int) vmResult);
            return MPI_ERR_NO_MEM;
        }

        programBuffer = (u8 *) uncompProgRange.virt;
        assert(programBuffer != NULL);

        // Decompress entire file
        const Lz4Decompressor::Result decompResult = lz4.read(programBuffer, lz4.getUncompressedSize());
        if (decompResult != Lz4Decompressor::Success)
        {
            ERROR("failed to decompress program buffer: result = " << (int) decompResult);
            return MPI_ERR_NO_MEM;
        }

        // Allocate memory space on the local processor for the whole
        // UniChannel array, NxN communication with MPI.
        // Then pass the channel offset physical address as an argument -addr 0x.... to spawn()
        memChannelBase.size = (PAGESIZE * 2) * (coreCount * coreCount);
        memChannelBase.phys = 0;
        memChannelBase.virt = 0;
        memChannelBase.access = Memory::Readable | Memory::Writable | Memory::User;
        vmResult = VMCtl(SELF, MapContiguous, &memChannelBase);
        if (vmResult != API::Success)
        {
            ERROR("failed to allocate MemoryChannel: result = " << (int) vmResult);
            return MPI_ERR_NO_MEM;
        }

        DEBUG("MemoryChannel at physical address " << (void *) memChannelBase.phys);

        // Clear channel pages
        MemoryBlock::set((void *) memChannelBase.virt, 0, memChannelBase.size);

        // Format program command with MPI specific arguments
        programCmd << programPath << " " << Number::Hex << (void *)(memChannelBase.phys) <<
                                     " " << Number::Dec << coreCount;

        // Append additional user arguments
        for (int j = 1; j < *argc; j++)
        {
            programCmd << " " << (*argv)[j];
        }

        // now create the slaves using coreservers.
        for (Size i = 1; i < coreCount; i++)
        {
            const Core::Result result = coreClient.createProcess(i, (const Address) programBuffer,
                                                                 lz4.getUncompressedSize(), *programCmd);
            if (result != Core::Success)
            {
                ERROR("failed to create process on core" << i << ": result = " << (int) result);
                return MPI_ERR_SPAWN;
            }
        }
    }
    else
    {
        // If we are slave (node N): read the memory channel base, corecount arguments
        if ((*argc) < 3)
        {
            ERROR("invalid number of arguments given");
            return MPI_ERR_ARG;
        }

        String s = (*argv)[1];
        memChannelBase.phys = s.toLong(Number::Hex);
        s = (*argv)[2];
        coreCount = s.toLong(Number::Dec);

        // Pass the rest of the arguments to the user program
        (*argc) -= 2;
        (*argv)[1] = (*argv)[0];
        (*argv) += 2;
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
            DEBUG("readChannel: core" << i << ": data = " << (void *) (MEMBASE(info.coreId) + (PAGESIZE * 2 * i)) <<
                  " feedback = " << (void *) (MEMBASE(info.coreId) + (PAGESIZE * 2 * i) + PAGESIZE) <<
                  " base" << i << " = " << (void *) (MEMBASE(i)));
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
            DEBUG("writeChannel: core" << i << ": data = " << (void *) (MEMBASE(i) + (PAGESIZE * 2 * info.coreId)) <<
                  " feedback = " << (void *) (MEMBASE(i) + (PAGESIZE * 2 * info.coreId) + PAGESIZE) <<
                  " base" << i << " = " << (void *) (MEMBASE(i)));
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
