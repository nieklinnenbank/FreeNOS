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
    if ((*argc) >= 5 && MemoryBlock::compare((*argv)[1], "--slave"))
    {
        return initializeSlave(argc, argv);
    }
    else
    {
        return initializeMaster(argc, argv);
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
        {
            ProcessCtl(SELF, Schedule, 0);
        }
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
        {
            ProcessCtl(SELF, Schedule, 0);
        }

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

MpiTarget::Result MpiTarget::initializeMaster(int *argc,
                                              char ***argv)
{
    const CoreClient coreClient;
    char *programName = (*argv)[0];
    String programPath;
    u8 *programBuffer;

    // We are the master node
    m_coreId = 0;

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

    // Allocate memory space for two-way communication
    // between the master and the other processors
    m_memChannelBase.size = (PAGESIZE * 2) * (m_coreCount) * 2;
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

    // now create the slaves using coreservers.
    for (Size i = 1; i < m_coreCount; i++)
    {
        String programCmd;

        // Format program command with MPI specific arguments for the slaves
        programCmd << programPath << " --slave " <<
            Number::Hex << (void *)(m_memChannelBase.phys) << " " <<
            Number::Dec << i << " " << m_coreCount;

        // Append additional user arguments
        for (int j = 1; j < *argc; j++)
        {
            programCmd << " " << (*argv)[j];
        }

        const Core::Result result = coreClient.createProcess(i, (const Address) programBuffer,
                                                             lz4.getUncompressedSize(), *programCmd);
        if (result != Core::Success)
        {
            ERROR("failed to create process on core" << i << ": result = " << (int) result);
            return MPI_ERR_SPAWN;
        }
    }

    // Fill read channels
    for (Size i = 1; i < m_coreCount; i++)
    {
        const Result readResult = createReadChannel(i, getMemoryBaseRead(i));
        if (readResult != MPI_SUCCESS)
        {
            ERROR("failed to create read MemoryChannel for core" << i << ": result = " << (int) readResult);
            return readResult;
        }
    }

    // Fill write channels
    for (Size i = 1; i < m_coreCount; i++)
    {
        const Result writeResult = createWriteChannel(i, getMemoryBaseWrite(i));
        if (writeResult != MPI_SUCCESS)
        {
            ERROR("failed to create write MemoryChannel for core" << i << ": result = " << (int) writeResult);
            return writeResult;
        }
    }

    return MPI_SUCCESS;
}

MpiTarget::Result MpiTarget::initializeSlave(int *argc,
                                             char ***argv)
{
    // If we are slave (node N): read arguments: --slave [memoryChannelBase] [rankId] [coreCount]
    if ((*argc) < 5)
    {
        ERROR("invalid number of arguments given");
        return MPI_ERR_ARG;
    }

    String s = (*argv)[2];
    m_memChannelBase.phys = s.toLong(Number::Hex);

    s = (*argv)[3];
    m_coreId = s.toLong(Number::Dec);
    assert(m_coreId != 0);

    s = (*argv)[4];
    m_coreCount = s.toLong(Number::Dec);
    assert(m_coreId < m_coreCount);
    assert(m_coreCount > 0);
    assert(m_coreCount <= MaximumChannels);

    // Pass the rest of the arguments to the user program
    (*argc) -= 4;
    (*argv)[4] = (*argv)[0];
    (*argv) += 4;

    // Create MemoryChannels for communication with the master
    const Result readResult = createReadChannel(0, getMemoryBaseRead(m_coreId));
    if (readResult != MPI_SUCCESS)
    {
        ERROR("failed to create read MemoryChannel for master: result = " << (int) readResult);
        return readResult;
    }

    const Result writeResult = createWriteChannel(0, getMemoryBaseWrite(m_coreId));
    if (writeResult != MPI_SUCCESS)
    {
        ERROR("failed to create write MemoryChannel for master: result = " << (int) writeResult);
        return writeResult;
    }

    return MPI_SUCCESS;
}

MpiTarget::Result MpiTarget::createReadChannel(const Size coreId,
                                               const Address memoryBase)
{
    MemoryChannel *ch = new MemoryChannel(Channel::Consumer, sizeof(MPIMessage));
    if (!ch)
    {
        ERROR("failed to allocate consumer MemoryChannel for coreId = " << coreId);
        return MPI_ERR_NO_MEM;
    }

    ch->setPhysical(memoryBase, memoryBase + PAGESIZE);
    m_readChannels.insertAt(coreId, ch);

    if (m_coreId == 0)
    {
        DEBUG(m_coreId << ": readChannel: core" << coreId << ": data = " << (void *) memoryBase <<
              " feedback = " << (void *) (memoryBase + PAGESIZE));
    }

    return MPI_SUCCESS;
}

MpiTarget::Result MpiTarget::createWriteChannel(const Size coreId,
                                                const Address memoryBase)
{
    MemoryChannel *ch = new MemoryChannel(Channel::Producer, sizeof(MPIMessage));
    if (!ch)
    {
        ERROR("failed to allocate producer MemoryChannel for coreId = " << coreId);
        return MPI_ERR_NO_MEM;
    }

    ch->setPhysical(memoryBase, memoryBase + PAGESIZE);
    m_writeChannels.insertAt(coreId, ch);

    if (m_coreId == 0)
    {
        DEBUG(m_coreId << ": writeChannel: core" << coreId << ": data = " << (void *) memoryBase <<
              " feedback = " << (void *) (memoryBase + PAGESIZE));
    }

    return MPI_SUCCESS;
}

Address MpiTarget::getMemoryBaseRead(const Size coreId) const
{
    assert(coreId < m_coreCount);

    const Address base = m_memChannelBase.phys + (PAGESIZE * 2 * (coreId));

    if (m_coreId == 0)
    {
        return base;
    }
    else
    {
        return base + (PAGESIZE * 2 * m_coreCount);
    }
}

Address MpiTarget::getMemoryBaseWrite(const Size coreId) const
{
    assert(coreId < m_coreCount);

    const Address base = m_memChannelBase.phys + (PAGESIZE * 2 * (coreId));

    if (m_coreId == 0)
    {
        return base + (PAGESIZE * 2 * m_coreCount);
    }
    else
    {
        return base;
    }
}
