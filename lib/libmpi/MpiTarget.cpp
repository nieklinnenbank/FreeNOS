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

#include <FreeNOS/User.h>
#include <MemoryChannel.h>
#include <CoreClient.h>
#include <Lz4Decompressor.h>
#include <Index.h>
#include <String.h>
#include <MemoryBlock.h>
#include <BufferedFile.h>
#include <MemoryChannel.h>
#include <Index.h>
#include <Log.h>
#include "MPIMessage.h"
#include "MpiTarget.h"


template<> MpiBackend* AbstractFactory<MpiBackend>::create()
{
    return new MpiTarget();
}

MpiTarget::MpiTarget()
    : m_coreId(0)
    , m_coreCount(0)
{
    MemoryBlock::set(&m_memChannelBase, 0, sizeof(m_memChannelBase));
}

MpiTarget::Result MpiTarget::initialize(int *argc,
                                        char ***argv)
{
    const CoreClient coreClient;
    char *programName = (*argv)[0];
    String programPath;
    String programCmd;
    u8 *programBuffer;

    // Retrieve our core identifier
    SystemInformation info;
    m_coreId = info.coreId;

    // If we are master (node 0):
    if (m_coreId == 0)
    {
        // Retrieve number of cores on the system
        const Core::Result result = coreClient.getCoreCount(m_coreCount);
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
        m_memChannelBase.size = (PAGESIZE * 2) * (m_coreCount * m_coreCount);
        m_memChannelBase.phys = 0;
        m_memChannelBase.virt = 0;
        m_memChannelBase.access = Memory::Readable | Memory::Writable | Memory::User;
        vmResult = VMCtl(SELF, MapContiguous, &m_memChannelBase);
        if (vmResult != API::Success)
        {
            ERROR("failed to allocate MemoryChannel: result = " << (int) vmResult);
            return MPI_ERR_NO_MEM;
        }

        DEBUG("MemoryChannel at physical address " << (void *) m_memChannelBase.phys);

        // Clear channel pages
        MemoryBlock::set((void *) m_memChannelBase.virt, 0, m_memChannelBase.size);

        // Format program command with MPI specific arguments
        programCmd << programPath << " " << Number::Hex << (void *)(m_memChannelBase.phys) <<
                                     " " << Number::Dec << m_coreCount;

        // Append additional user arguments
        for (int j = 1; j < *argc; j++)
        {
            programCmd << " " << (*argv)[j];
        }

        // now create the slaves using coreservers.
        for (Size i = 1; i < m_coreCount; i++)
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
        m_memChannelBase.phys = s.toLong(Number::Hex);
        s = (*argv)[2];
        m_coreCount = s.toLong(Number::Dec);

        // Pass the rest of the arguments to the user program
        (*argc) -= 2;
        (*argv)[1] = (*argv)[0];
        (*argv) += 2;
    }

    // Create MemoryChannels
    assert(m_coreCount > 0);
    assert(m_coreCount <= MaximumChannels);

    // Fill read channels
    for (Size i = 0; i < m_coreCount; i++)
    {
        MemoryChannel *ch = new MemoryChannel(Channel::Consumer, sizeof(MPIMessage));
        assert(ch != NULL);
        ch->setPhysical(getMemoryBase(m_coreId) + (PAGESIZE * 2 * i),
                        getMemoryBase(m_coreId) + (PAGESIZE * 2 * i) + PAGESIZE);
        m_readChannels.insertAt(i, ch);

        if (m_coreId == 0)
        {
            DEBUG("readChannel: core" << i << ": data = " << (void *) (getMemoryBase(m_coreId) + (PAGESIZE * 2 * i)) <<
                  " feedback = " << (void *) (getMemoryBase(m_coreId) + (PAGESIZE * 2 * i) + PAGESIZE) <<
                  " base" << i << " = " << (void *) (getMemoryBase(i)));
        }
    }

    // Fill write channels
    for (Size i = 0; i < m_coreCount; i++)
    {
        MemoryChannel *ch = new MemoryChannel(Channel::Producer, sizeof(MPIMessage));
        assert(ch != NULL);
        ch->setPhysical(getMemoryBase(i) + (PAGESIZE * 2 * m_coreId),
                        getMemoryBase(i) + (PAGESIZE * 2 * m_coreId) + PAGESIZE);
        m_writeChannels.insertAt(i, ch);

        if (m_coreId == 0)
        {
            DEBUG("writeChannel: core" << i << ": data = " << (void *) (getMemoryBase(i) + (PAGESIZE * 2 * m_coreId)) <<
                  " feedback = " << (void *) (getMemoryBase(i) + (PAGESIZE * 2 * m_coreId) + PAGESIZE) <<
                  " base" << i << " = " << (void *) (getMemoryBase(i)));
        }
    }

    return MPI_SUCCESS;
}

MpiTarget::Result MpiTarget::terminate()
{
    return MPI_SUCCESS;
}

MpiTarget::Result MpiTarget::getCommRank(MPI_Comm comm,
                                         int *rank)
{
    *rank = m_coreId;
    return MPI_SUCCESS;
}

MpiTarget::Result MpiTarget::getCommSize(MPI_Comm comm,
                                         int *size)
{
    *size = m_coreCount;
    return MPI_SUCCESS;
}

MpiTarget::Result MpiTarget::send(const void *buf,
                                  int count,
                                  MPI_Datatype datatype,
                                  int dest,
                                  int tag,
                                  MPI_Comm comm)
{
    MPIMessage msg;
    MemoryChannel *ch;

    if (!(ch = m_writeChannels.get(dest)))
    {
        return MPI_ERR_RANK;
    }

    for (int i = 0; i < count; i++)
    {
        switch (datatype)
        {
            case MPI_INT:
                msg.integer = *(((int *) buf) + i);
                break;

            case MPI_UNSIGNED_CHAR:
                msg.uchar = *(((u8 *) buf) + i);
                break;

            default:
                return MPI_ERR_UNSUPPORTED_DATAREP;
        }

        while (ch->write(&msg) != Channel::Success)
            ;
    }

    return MPI_SUCCESS;
}

MpiTarget::Result MpiTarget::receive(void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     int source,
                                     int tag,
                                     MPI_Comm comm,
                                     MPI_Status *status)
{
    MPIMessage msg;
    MemoryChannel *ch;

    if (!(ch = m_readChannels.get(source)))
    {
        return MPI_ERR_RANK;
    }

    for (int i = 0; i < count; i++)
    {
        while (ch->read(&msg) != Channel::Success)
            ;

        switch (datatype)
        {
            case MPI_INT:
                *(((int *) buf) + i) = msg.integer;
                break;

            case MPI_UNSIGNED_CHAR:
                *(((u8 *) buf) + i) = msg.uchar;
                break;

            default:
                return MPI_ERR_UNSUPPORTED_DATAREP;
        }
    }

    return MPI_SUCCESS;
}

Address MpiTarget::getMemoryBase(const Size coreId) const
{
    assert(coreId < m_coreCount);

    return m_memChannelBase.phys + (m_coreCount * PAGESIZE * 2 * (coreId));
}
