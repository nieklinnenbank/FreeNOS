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

#include <FreeNOS/System.h>
#include <Log.h>
#include <IPV4.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <MemoryBlock.h>
#include <MemoryChannel.h>
#include <FileSystemPath.h>
#include <NetworkClient.h>
#include <NetworkQueue.h>
#include <BufferedFile.h>
#include <MPIMessage.h>
#include <ApplicationLauncher.h>
#include <Lz4Decompressor.h>
#include <CoreClient.h>
#include <mpi.h>
#include "MpiProxy.h"

#pragma clang optimize off
#pragma GCC push_options
#pragma GCC optimize ("O0")

MpiProxy::MpiProxy(int argc, char **argv)
    : POSIXApplication(argc, argv)
    , m_sock(-1)
    , m_client(ZERO)
{
    MemoryBlock::set(&m_memChannelBase, 0, sizeof(m_memChannelBase));
    m_pids.fill(ANY);

    parser().setDescription("Message Passing Interface (MPI) proxy server");
    parser().registerPositional("DEVICE", "device name of network adapter");
}

MpiProxy::~MpiProxy()
{
    DEBUG("");

    if (m_client != ZERO)
    {
        delete m_client;
    }
}

MpiProxy::Result MpiProxy::initialize()
{
    const char *device = arguments().get("DEVICE");
    NetworkClient::Result result;

    DEBUG("");

    // Create a network client
    m_client = new NetworkClient(device);
    if (m_client == ZERO)
    {
        ERROR("failed to allocate NetworkClient");
        return IOError;
    }

    // Initialize networking client
    result = m_client->initialize();
    if (result != NetworkClient::Success)
    {
        ERROR("failed to initialize network client for device "
               << device << ": result = " << (int) result);
        return IOError;
    }

    // Create an UDP socket
    result = m_client->createSocket(NetworkClient::UDP, &m_sock);
    if (result != NetworkClient::Success)
    {
        ERROR("failed to create UDP socket on device " << device <<
              ": result = " << (int) result);
        return IOError;
    }

    // Bind to a local port.
    result = m_client->bindSocket(m_sock, 0, UdpPort);
    if (result != NetworkClient::Success)
    {
        ERROR("failed to bind socket to UDP port " << UdpPort <<
              " on device " << device << ": result = " << (int) result);
        return IOError;
    }

    return Success;
}

MpiProxy::Result MpiProxy::exec()
{
    DEBUG("");

    while (true)
    {
        static u8 packet[MaximumPacketSize];
        Size packetSize = sizeof(packet);
        struct sockaddr addr;

        // Wait for UDP packet
        const Result recvResult = udpReceive(packet, packetSize, addr);
        if (recvResult != Success)
        {
            if (recvResult != TimedOut)
            {
                ERROR("failed to receive UDP packet: result = " << (int) recvResult);
            }
            continue;
        }

        // Process the packet
        const Result procResult = processRequest(packet, packetSize, addr);
        if (procResult != Success)
        {
            ERROR("failed to process UDP packet: result = " << (int) procResult);
            continue;
        }
    }

    return Success;
}

MpiProxy::Result MpiProxy::udpSend(const void *packet,
                                   const Size size,
                                   const struct sockaddr & addr) const
{
    DEBUG("size = " << size);

    // Send the packet
    int result = ::sendto(m_sock, packet, size, 0,
                          &addr, sizeof(addr));
    if (result <= 0)
    {
        ERROR("failed to send UDP datagram: " << strerror(errno));
        return IOError;
    }

    return Success;
}

MpiProxy::Result MpiProxy::udpSendMultiple(const struct iovec *vec,
                                           const Size count,
                                           const struct sockaddr & addr) const
{
    struct msghdr msg;

    DEBUG("count = " << count);

    // Prepare the message header
    msg.msg_name = (void *) &addr;
    msg.msg_namelen = sizeof(addr);
    msg.msg_iov = (struct iovec *) vec;
    msg.msg_iovlen = count;

    // Send the packet
    int result = ::sendmsg(m_sock, &msg, 0);
    if (result <= 0)
    {
        ERROR("failed to send multiple UDP datagrams: " << strerror(errno));
        return IOError;
    }

    return Success;
}

MpiProxy::Result MpiProxy::udpReceive(void *packet,
                                      Size & size,
                                      struct sockaddr & addr) const
{
    DEBUG("");

    // Wait for a packet in the UDP socket
    const NetworkClient::Result result = m_client->waitSocket(NetworkClient::UDP, m_sock, ReceiveTimeoutMs);
    if (result != NetworkClient::Success)
    {
        if (result == NetworkClient::TimedOut)
        {
            return TimedOut;
        }
        else
        {
            ERROR("failed to wait for UDP socket " << m_sock);
            return IOError;
        }
    }

    // Receive UDP datagram
    int r = recvfrom(m_sock, packet, size, 0,
                     &addr, sizeof(addr));
    if (r < 0)
    {
        ERROR("failed to receive UDP datagram: " << strerror(errno));
        return IOError;
    }

    size = r;
    DEBUG("received " << r << " bytes from " << *IPV4::toString(addr.addr) <<
          " at port " << addr.port);

    return Success;
}

MpiProxy::Result MpiProxy::processRequest(const u8 *packet,
                                          const Size size,
                                          const struct sockaddr & addr)
{
    const Header *hdr = (const Header *)(packet);

    DEBUG("hdr->operation = " << hdr->operation << " size = " << size);

    switch (hdr->operation)
    {
        case MpiOpSend:
            return processSend(hdr, packet, size);

        case MpiOpRecv:
            return processRecv(hdr, packet, size, addr);

        case MpiOpExec:
            return processExec(hdr, packet, size, addr);

        case MpiOpTerminate:
            return processTerminate(hdr, packet, size, addr);

        default:
            ERROR("unknown operation: " << Number::Hex << hdr->operation);
            break;
    }

    return Success;
}

MpiProxy::Result MpiProxy::processSend(const Header *header,
                                       const u8 *packet,
                                       const Size size)
{
    MPIMessage msg;
    MemoryChannel *ch;
    const u8 *buf = (packet + sizeof(*header));

    NOTICE("rankId = " << header->rankId << " datatype = " <<
           header->datatype << " datacount = " << header->datacount);

    if (!(ch = m_writeChannels.get(header->rankId)))
    {
        ERROR("rankId " << header->rankId << " not found");
        return NotFound;
    }

    for (Size i = 0; i < header->datacount; i++)
    {
        switch (header->datatype)
        {
            case MPI_INT:
                msg.integer = *(((int *) buf) + i);
                break;

            case MPI_UNSIGNED_CHAR:
                msg.uchar = *(((u8 *) buf) + i);
                break;

            default:
            {
                ERROR("unsupported datatype = " << header->datatype);
                return NotFound;
            }
        }

        while (ch->write(&msg) != Channel::Success)
            ;
    }

    return Success;
}

MpiProxy::Result MpiProxy::processRecv(const Header *header,
                                       const u8 *packet,
                                       const Size size,
                                       const struct sockaddr & addr)
{
    MemoryChannel *ch;
    static u8 pkts[NetworkQueue::MaxPackets][NetworkQueue::PayloadBufferSize];
    static struct iovec vec[NetworkQueue::MaxPackets];
    Size packetCount = 0;

    assert(NetworkQueue::PayloadBufferSize >= MaximumPacketSize);

    NOTICE("rankId = " << header->rankId << " datatype = " <<
           header->datatype << " datacount = " << header->datacount);

    if (!(ch = m_readChannels.get(header->rankId)))
    {
        ERROR("rankId " << header->rankId << " not found");
        return NotFound;
    }

    // Read from the channel and send out packet(s)
    for (Size i = 0; i < header->datacount;)
    {
        MPIMessage msg;
        Header *hdr = (Header *) pkts[packetCount];
        u8 *buf = (u8 *)(hdr + 1);
        Size pktSize = sizeof(Header);

        // Prepare header
        hdr->operation = MpiOpRecv;
        hdr->result = MPI_SUCCESS;
        hdr->coreId = header->coreId;
        hdr->rankId = header->rankId;
        hdr->datatype = header->datatype;
        hdr->datacount = 0;

        while (pktSize < MaximumPacketSize && i < header->datacount)
        {
            while (ch->read(&msg) != Channel::Success)
            {
                ProcessCtl(SELF, Schedule, 0);
            }

            switch (header->datatype)
            {
                case MPI_INT:
                    *(((int *) buf) + hdr->datacount) = msg.integer;
                    pktSize += sizeof(int);
                    break;

                case MPI_UNSIGNED_CHAR:
                    *(((u8 *) buf) + hdr->datacount) = msg.uchar;
                    pktSize += sizeof(u8);
                    break;

                default:
                {
                    ERROR("unsupported datatype = " << header->datatype);
                    return NotFound;
                }
            }
            // Move to the next data item
            i++;
            pktSize++;
            hdr->datacount++;
        }

        // Fill the I/O vector struct
        vec[packetCount].iov_base = (void *) hdr;
        vec[packetCount].iov_len = pktSize;
        packetCount++;

        if (hdr->datacount != 0 && (packetCount == NetworkQueue::MaxPackets || i >= header->datacount))
        {
            // UDP send
            const Result sendResult = udpSendMultiple(vec, packetCount, addr);
            if (sendResult != Success)
            {
                ERROR("failed to send multiple UDP packets: result = " << (int) sendResult);
                return sendResult;
            }
            packetCount = 0;
        }
    }

    return Success;
}

MpiProxy::Result MpiProxy::processExec(const Header *header,
                                       const u8 *packet,
                                       const Size size,
                                       const struct sockaddr & addr)
{
    char cmd[FileSystemPath::MaximumLength + 1];
    Result result = Success;

    MemoryBlock::set(cmd, 0, sizeof(cmd));
    MemoryBlock::copy(cmd, header + 1, size - sizeof(*header));
    cmd[FileSystemPath::MaximumLength] = 0;

    DEBUG("exec: cmd = '" << cmd << "' rankId = " << header->rankId <<
          " coreId = " << header->coreId << " coreCount = " << header->coreCount);

    const Result chanResult = createChannels(header->rankId, header->coreCount);
    if (chanResult != Success)
    {
        ERROR("failed to create MPI communication channels for rankId = " << header->rankId <<
              " result = " << (int) chanResult);
        return chanResult;
    }

    if (header->coreId == 0)
    {
        result = startLocalProcess(cmd, header->rankId, header->coreCount);
    }
    else
    {
        result = startRemoteProcess(header->coreId, cmd, header->rankId, header->coreCount);
    }

    // Send acknowledge of start
    static u8 pkt[MaximumPacketSize];
    Header *hdr = (Header *) pkt;
    hdr->operation = MpiOpExec;
    hdr->result = result == Success ? MPI_SUCCESS : MPI_ERR_IO;
    hdr->coreId = header->coreId;
    hdr->rankId = header->rankId;
    Size pktSize = sizeof(*hdr);

    const Result sendResult = udpSend(pkt, pktSize, addr);
    if (sendResult != Success)
    {
        ERROR("failed to send UDP packet: result = " << (int) sendResult);
        return sendResult;
    }

    return result;
}

MpiProxy::Result MpiProxy::processTerminate(const Header *header,
                                            const u8 *packet,
                                            const Size size,
                                            const struct sockaddr & addr)
{
    static u8 pkt[MaximumPacketSize];
    Header *hdr = (Header *) pkt;
    Size pktSize = sizeof(Header);

    NOTICE("size = " << size);

    // Loop PIDs of active processes and wait for each to terminate
    for (Size i = 0; i < m_pids.size(); i++)
    {
        if (m_pids[i] != ANY)
        {
            if (i == 0)
            {
                int status;
                waitpid(m_pids[i], &status, 0);
            }
            else
            {
                // TODO: send a CoreClient::ping to remote cores,
                // to check if they are finished with the remote program
            }

            m_pids.insert(i, ANY);
        }
    }

    // Release m_memChannelBase here
    if (m_memChannelBase.virt != ZERO)
    {
        const API::Result releaseResult = VMCtl(SELF, Release, &m_memChannelBase);
        if (releaseResult != API::Success)
        {
            ERROR("failed to release memory of communication channels: result = " << (int) releaseResult);
            return IOError;
        }

        MemoryBlock::set(&m_memChannelBase, 0, sizeof(m_memChannelBase));
    }

    // Prepare header for response
    hdr->operation = MpiOpTerminate;
    hdr->result = MPI_SUCCESS;
    hdr->rankId = header->rankId;
    hdr->coreId = header->coreId;

    // UDP send
    const Result sendResult = udpSend(pkt, pktSize, addr);
    if (sendResult != Success)
    {
        ERROR("failed to send UDP packet: result = " << (int) sendResult);
        return sendResult;
    }

    return Success;
}

MpiProxy::Result MpiProxy::createChannels(const Size rankId,
                                          const Size coreCount)
{
    DEBUG("rankId = " << rankId << " coreCount = " << coreCount);

    // Allocate memory space for two-way communication
    // between the proxy server and the other processes
    if (m_memChannelBase.virt == ZERO)
    {
        m_memChannelBase.size = (PAGESIZE * 2) * coreCount * 2;
        m_memChannelBase.phys = 0;
        m_memChannelBase.virt = 0;
        m_memChannelBase.access = Memory::Readable | Memory::Writable | Memory::User;
        const API::Result vmResult = VMCtl(SELF, MapContiguous, &m_memChannelBase);
        if (vmResult != API::Success)
        {
            ERROR("failed to allocate MemoryChannel: result = " << (int) vmResult);
            return OutOfMemory;
        }

        // Clear channel pages
        MemoryBlock::set((void *) m_memChannelBase.virt, 0, m_memChannelBase.size);
        NOTICE("MemoryChannel at physical address " << (void *) m_memChannelBase.phys);
    }

    // Create read channel
    if (m_readChannels.get(rankId) == ZERO)
    {
        MemoryChannel *ch = new MemoryChannel(Channel::Consumer, sizeof(MPIMessage));
        if (!ch)
        {
            ERROR("failed to allocate consumer MemoryChannel for rankId = " << rankId);
            return OutOfMemory;
        }

        m_readChannels.insertAt(rankId, ch);
    }
    else
    {
        const MemoryChannel::Result unmapResult = m_readChannels.get(rankId)->unmap();
        if (unmapResult != MemoryChannel::Success)
        {
            ERROR("failed to unmap read MemoryChannel: result = " << (int) unmapResult);
            return IOError;
        }
    }

    const Address readMemoryBase = m_memChannelBase.phys + (PAGESIZE * 2 * rankId);
    m_readChannels.get(rankId)->setPhysical(readMemoryBase, readMemoryBase + PAGESIZE);

    NOTICE("readChannel: rank" << rankId << ": data = " << (void *) readMemoryBase <<
          " feedback = " << (void *) (readMemoryBase + PAGESIZE));

    // Create write channel
    if (m_writeChannels.get(rankId) == ZERO)
    {
        MemoryChannel *ch = new MemoryChannel(Channel::Producer, sizeof(MPIMessage));
        if (!ch)
        {
            ERROR("failed to allocate producer MemoryChannel for rankId = " << rankId);
            return OutOfMemory;
        }

        m_writeChannels.insertAt(rankId, ch);
    }
    else
    {
        const MemoryChannel::Result unmapResult = m_writeChannels.get(rankId)->unmap();
        if (unmapResult != MemoryChannel::Success)
        {
            ERROR("failed to unmap write MemoryChannel: result = " << (int) unmapResult);
            return IOError;
        }
    }

    const Address writeMemoryBase = m_memChannelBase.phys + (PAGESIZE * 2 * coreCount) + (PAGESIZE * 2 * rankId);
    m_writeChannels.get(rankId)->setPhysical(writeMemoryBase, writeMemoryBase + PAGESIZE);

    NOTICE("writeChannel: rank" << rankId << ": data = " << (void *) writeMemoryBase <<
           " feedback = " << (void *) (writeMemoryBase + PAGESIZE));

    return Success;
}

MpiProxy::Result MpiProxy::startLocalProcess(const char *command,
                                             const Size rankId,
                                             const Size coreCount)
{
    DEBUG("command = '" << command << "' rankId = " << rankId <<
          " coreCount = " << coreCount);

    List<String> programArgs = String(command).split(' ');
    String programPath;

    // Prepare full path to the program to start
    programPath << "/bin/" << *programArgs[0];

    // Format program arguments with MPI specific arguments for the slaves
    String programCmd;
    programCmd << programPath << " --slave " <<
        Number::Hex << (void *)(m_memChannelBase.phys) << " " <<
        Number::Dec << rankId << " " << coreCount;

    // Append additional user arguments
    ListIterator<String> it(programArgs);
    it++;
    for (; it.hasCurrent(); it++)
    {
        programCmd << " " << (*it.current());
    }

    NOTICE("programCmd = '" << *programCmd << "'");

    // Convert full programCmd to argc/argv format for ApplicationLauncher
    List<String> fullProgramArgs = programCmd.split(' ');
    char **argv = new char*[fullProgramArgs.count() + 1];
    int argc = 0;

    for (ListIterator<String> i(fullProgramArgs); i.hasCurrent(); i++)
    {
        argv[argc++] = *i.current();

        NOTICE("argv[" << (argc-1) << "] = " << argv[argc-1]);
    }

    // Terminate argv array with zero
    argv[fullProgramArgs.count()] = ZERO;

    // Start the local process on this core
    ApplicationLauncher launcher(*programPath, (const char **) argv);

    const ApplicationLauncher::Result execResult = launcher.exec();
    if (execResult != ApplicationLauncher::Success)
    {
        ERROR("failed to start program on local core: result = " << (int) execResult);
        delete[] argv;
        return IOError;
    }

    NOTICE("started with PID = " << launcher.getPid());
    m_pids.insert(0, launcher.getPid());

    delete[] argv;
    return Success;
}

MpiProxy::Result MpiProxy::startRemoteProcess(const Size coreId,
                                              const char *command,
                                              const Size rankId,
                                              const Size coreCount)
{
    DEBUG("coreId = " << coreId << " command = '" << command <<
          "' rankId = " << rankId << " coreCount = " << coreCount);

    const CoreClient coreClient;
    List<String> programArgs = String(command).split(' ');
    String programPath;

    // Prepare full path to the program to start
    programPath << "/bin/" << *programArgs[0];

    // Try to read the raw ELF program data (compressed)
    BufferedFile programFile(*programPath);
    const BufferedFile::Result readResult = programFile.read();
    if (readResult != BufferedFile::Success)
    {
        ERROR("failed to read program at path '" << *programPath <<
              "': result = " << (int) readResult);
        return NotFound;
    }

    // Initialize decompressor
    Lz4Decompressor lz4(programFile.buffer(), programFile.size());
    Lz4Decompressor::Result lz4Result = lz4.initialize();
    if (lz4Result != Lz4Decompressor::Success)
    {
        ERROR("failed to initialize LZ4 decompressor: result = " << (int) lz4Result);
        return IOError;
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
        return OutOfMemory;
    }

    u8 *programBuffer = (u8 *) uncompProgRange.virt;
    assert(programBuffer != NULL);

    // Decompress entire file
    const Lz4Decompressor::Result decompResult = lz4.read(programBuffer, lz4.getUncompressedSize());
    if (decompResult != Lz4Decompressor::Success)
    {
        ERROR("failed to decompress program buffer: result = " << (int) decompResult);
        return IOError;
    }

    // Format program command with MPI specific arguments for the slaves
    String programCmd;
    programCmd << programPath << " --slave " <<
        Number::Hex << (void *)(m_memChannelBase.phys) << " " <<
        Number::Dec << rankId << " " << coreCount;

    // Append additional user arguments
    ListIterator<String> i(programArgs);
    i++;
    for (; i.hasCurrent(); i++)
    {
        programCmd << " " << (*i.current());
    }

    DEBUG("programCmd = '" << *programCmd << "'");

    // Start the program on the secondary core
    const Core::Result result = coreClient.createProcess(coreId, (const Address) programBuffer,
                                                         lz4.getUncompressedSize(), *programCmd);
    if (result != Core::Success)
    {
        ERROR("failed to create process on core" << coreId << ": result = " << (int) result);
        return IOError;
    }

    // Cleanup uncompressed program buffer
    const API::Result releaseResult = VMCtl(SELF, Release, &uncompProgRange);
    if (releaseResult != API::Success)
    {
        ERROR("failed to release memory of uncompressed program: result = " << (int) releaseResult);
        return IOError;
    }

    // PID of remote processes are not currently visible
    m_pids.insert(coreId, SELF);
    return Success;
}
