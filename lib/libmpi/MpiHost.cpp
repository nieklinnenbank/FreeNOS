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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <Log.h>
#include <List.h>
#include <ListIterator.h>
#include <String.h>
#include <MpiProxy.h>
#include "MpiHost.h"

template<> MpiBackend* AbstractFactory<MpiBackend>::create()
{
    return new MpiHost();
}

MpiHost::MpiHost()
{
}

MpiHost::Result MpiHost::initialize(int *argc,
                                    char ***argv)
{
    // Verify input arguments
    if ((*argc) < 2)
    {
        ERROR("invalid number of arguments given");
        return MPI_ERR_ARG;
    }

    // Add ourselves as the master node
    Node *master = new Node;
    master->ipAddress = 0;
    master->udpPort = 0;
    master->coreId = 0;

    // Register the master
    if (!m_nodes.insert(master))
    {
        ERROR("failed to add master Node object");
        return MPI_ERR_IO;
    }

    // Read list of hosts from the given file
    const Result hostsResult = parseHostsFile((*argv)[1]);
    if (hostsResult != MPI_SUCCESS)
    {
        ERROR("failed to parse hosts file at path " << (*argv)[1] <<
              ": result = " << (int) hostsResult);
        return hostsResult;
    }

    // Pass the rest of the arguments to the user program
    (*argc) -= 1;
    (*argv)[1] = (*argv)[0];
    (*argv) += 1;

    // Create UDP socket
    m_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_sock < 0)
    {
        ERROR("failed to create UDP socket: " << strerror(errno));
        return MPI_ERR_IO;
    }

    // Prepare local address and port to bind to
    struct sockaddr_in addr;
    memset((char *)&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(0);

    // Bind the UDP socket
    int bindResult = bind(m_sock, (struct sockaddr *) &addr, sizeof(addr));
    if (bindResult < 0)
    {
        ERROR("failed to bind UDP socket: " << strerror(errno));
        return MPI_ERR_IO;
    }

    // Launch remote programs
    return startProcesses(*argc, *argv);
}

MpiHost::Result MpiHost::terminate()
{
    // Loop all nodes
    for (Size i = 1; i < m_nodes.count(); i++)
    {
        static u8 packet[MpiProxy::MaximumPacketSize];
        Size packetSize = sizeof(MpiProxy::Header);

        // Send terminate request to the remote node
        MpiProxy::Header *hdr = (MpiProxy::Header *) packet;
        hdr->operation = MpiProxy::MpiOpTerminate;
        hdr->coreId = m_nodes[i]->coreId;
        hdr->rankId = i;

        // Send the packet
        const Result sendResult = sendPacket(i, packet, sizeof(MpiProxy::Header));
        if (sendResult != MPI_SUCCESS)
        {
            ERROR("failed to send packet to nodeId " << i << ": result = " << (int) sendResult);
            return sendResult;
        }

        // Wait for reply
        const Result recvResult = receivePacket(i, MpiProxy::MpiOpTerminate, packet, packetSize);
        if (recvResult != MPI_SUCCESS)
        {
            ERROR("failed to receive UDP packet for rankId = " << i << ": result = " << (int) recvResult);
            return recvResult;
        }

        // The packet must be a terminate response
        const MpiProxy::Header *header = (const MpiProxy::Header *) packet;
        if (header->operation != MpiProxy::MpiOpTerminate)
        {
            ERROR("invalid response received: op = " << header->operation);
            continue;
        }

        // Verify the result code
        if (header->result != MPI_SUCCESS)
        {
            ERROR("rankId " << i << " failed to terminate with result = " << header->result);
            continue;
        }

        NOTICE("rankId " << i << " terminated");
    }

    return MPI_SUCCESS;
}

MpiHost::Result MpiHost::getCommRank(MPI_Comm comm,
                                     int *rank)
{
    *rank = 0;
    return MPI_SUCCESS;
}

MpiHost::Result MpiHost::getCommSize(MPI_Comm comm,
                                     int *size)
{
    *size = m_nodes.count();
    return MPI_SUCCESS;
}

MpiHost::Result MpiHost::send(const void *buf,
                              int count,
                              MPI_Datatype datatype,
                              int dest,
                              int tag,
                              MPI_Comm comm)
{
    Size datasize = 0;

    // Get datatype size
    switch (datatype)
    {
        case MPI_INT:
            datasize = sizeof(int);
            break;

        case MPI_UNSIGNED_CHAR:
            datasize = sizeof(u8);
            break;

        default: {
            ERROR("unsupported datatype = " << (int) datatype);
            return MPI_ERR_ARG;
        }
    }

    // Large payloads are not yet supported
    if ((count * datasize) + sizeof(MpiProxy::Header) > MpiProxy::MaximumPacketSize)
    {
        ERROR("data count too high: maximum is " <<
              (MpiProxy::MaximumPacketSize - sizeof(MpiProxy::Header)));
        return MPI_ERR_ARG;
    }

    // Find the destination node
    const Node *node = m_nodes.get(dest);
    if (node == ZERO)
    {
        ERROR("nodeId " << dest << " not found");
        return MPI_ERR_ARG;
    }

    // Construct packet to send
    u8 packet[MpiProxy::MaximumPacketSize];
    MpiProxy::Header *hdr = (MpiProxy::Header *) packet;
    hdr->operation = MpiProxy::MpiOpSend;
    hdr->result = 0;
    hdr->coreId = node->coreId;
    hdr->rankId = dest;
    hdr->datatype = datatype;
    hdr->datacount = count;

    // Append payload after the header
    MemoryBlock::copy(packet + sizeof(MpiProxy::Header), buf, count * datasize);

    // Send the packet
    const Result sendResult = sendPacket(dest, packet, sizeof(MpiProxy::Header) + (count * datasize));
    if (sendResult != MPI_SUCCESS)
    {
        ERROR("failed to send packet to nodeId " << dest << ": result = " << (int) sendResult);
        return sendResult;
    }

    return MPI_SUCCESS;
}

MpiHost::Result MpiHost::receive(void *buf,
                                 int count,
                                 MPI_Datatype datatype,
                                 int source,
                                 int tag,
                                 MPI_Comm comm,
                                 MPI_Status *status)
{
    static u8 packet[MpiProxy::MaximumPacketSize];

    // Find the source node
    const Node *node = m_nodes.get(source);
    if (node == ZERO)
    {
        ERROR("nodeId " << source << " not found");
        return MPI_ERR_RANK;
    }

    // Send receive data request to the remote node
    MpiProxy::Header *hdr = (MpiProxy::Header *) packet;
    hdr->operation = MpiProxy::MpiOpRecv;
    hdr->coreId = node->coreId;
    hdr->rankId = source;
    hdr->datatype = datatype;
    hdr->datacount = count;

    // Send the packet
    const Result sendResult = sendPacket(source, packet, sizeof(MpiProxy::Header));
    if (sendResult != MPI_SUCCESS)
    {
        ERROR("failed to send packet to nodeId " << source << ": result = " << (int) sendResult);
        return sendResult;
    }

    // Now receive the data response(s)
    for (int i = 0; i < count;)
    {
        Size packetSize = sizeof(packet);

        // Receive data packet from the source node
        const Result recvResult = receivePacket(source, MpiProxy::MpiOpRecv, packet, packetSize);
        if (recvResult != MPI_SUCCESS)
        {
            ERROR("failed to receive UDP packet for rankId = " << source << ": result = " << (int) recvResult);
            return recvResult;
        }

        // The packet must be a data response
        const MpiProxy::Header *header = (const MpiProxy::Header *) packet;
        if (header->operation != MpiProxy::MpiOpRecv)
        {
            ERROR("invalid response received: op = " << header->operation);
            continue;
        }

        // Process all the received data
        for (Size j = 0; j < header->datacount; j++, i++)
        {
            const u8 *data = ((const u8 *)(header + 1)) + j;

            switch (datatype)
            {
                case MPI_INT:
                    *(((int *) buf) + i) = *(int *)(data);
                    break;

                case MPI_UNSIGNED_CHAR:
                    *(((u8 *) buf) + i) = *data;
                    break;

                default:
                    return MPI_ERR_UNSUPPORTED_DATAREP;
            }
        }
    }

    return MPI_SUCCESS;
}

MpiHost::Result MpiHost::parseHostsFile(const char *hostsfile)
{
    struct stat st;
    FILE *fp;

    DEBUG("hostsfile = " << hostsfile);

    if (stat(hostsfile, &st) != 0)
    {
        ERROR("failed to stat() `" << hostsfile << "': " << strerror(errno));
        return MPI_ERR_IO;
    }

    // Open file
    if ((fp = fopen(hostsfile, "r")) == NULL)
    {
        ERROR("failed to fopen() `" << hostsfile << "': " << strerror(errno));
        return MPI_ERR_IO;
    }

    // Allocate buffer storage
    char *contents = new char[st.st_size + 1];
    if (!contents)
    {
        ERROR("failed to allocate memory buffer for hostsfile: " << strerror(errno));
        return MPI_ERR_NO_MEM;
    }

    // Read the entire file into memory
    if (fread(contents, st.st_size, 1, fp) != (size_t) 1U)
    {
        ERROR("failed to fread() `" << hostsfile << "': " << strerror(errno));
        fclose(fp);
        return MPI_ERR_IO;
    }
    fclose(fp);

    // Null terminate
    contents[st.st_size] = 0;

    // Parse it into lines
    String contentString(contents);
    List<String> lines = contentString.split('\n');

    // Add each line as IP address of Execute each command
    for (ListIterator<String> i(lines); i.hasCurrent(); i++)
    {
        // Skip comment lines
        if (i.current()[0] == '#')
        {
            continue;
        }

        List<String> nodeLine = i.current().split(':');
        Size idx;

        // Nodes must be listed in the format: <ip>:<port>:<core>
        if (nodeLine.count() != 3)
        {
            ERROR("invalid host format '" << *i.current() << "' in hosts file at " << hostsfile);
            delete[] contents;
            return MPI_ERR_ARG;
        }

        Node *node = new Node();
        if (!node)
        {
            ERROR("failed to allocate Node object: " << strerror(errno));
            delete[] contents;
            return MPI_ERR_NO_MEM;
        }

        // Add the node
        node->ipAddress = inet_addr(*nodeLine[0]);
        node->udpPort = atoi(*nodeLine[1]);
        node->coreId = atoi(*nodeLine[2]);

        if (!m_nodes.insert(idx, node))
        {
            ERROR("failed to insert Node object");
            delete[] contents;
            return MPI_ERR_IO;
        }

        // Add packet buffer list
        List<Packet *> *lst = new List<Packet *>();
        if (!lst)
        {
            ERROR("failed to allocate List<..> object: " << strerror(errno));
            delete[] contents;
            return MPI_ERR_NO_MEM;
        }
        m_packetBuffers.insertAt(idx, lst);

        DEBUG("m_nodes[" << idx << "]: ip = " << *nodeLine[0] << ", port = " << *nodeLine[1] <<
              ", core = " << *nodeLine[2]);
    }

    // Cleanup
    delete[] contents;
    return MPI_SUCCESS;
}

MpiHost::Result MpiHost::startProcesses(int argc,
                                        char **argv)
{
    const Size NumOfParallelStart = 32;
    static u8 packet[MpiProxy::MaximumPacketSize];
    MpiProxy::Header *hdr = (MpiProxy::Header *) packet;
    String cmdline;
    Size startIndex = 1, startCount = 0;

    DEBUG("argc = " << argc);

    // First add the program name
    cmdline << basename(argv[0]);
    if (argc > 1)
    {
        cmdline << " ";
    }

    // Append any extra arguments
    for (int i = 1; i < argc; i++)
    {
        cmdline << argv[i];

        if (i != argc - 1)
        {
            cmdline << " ";
        }
    }

    // Start remote processes with the constructed command line
    NOTICE("cmdline = " << *cmdline);

    // Send out packets to all the hosts
    while (startIndex < m_nodes.count())
    {
        const Size receiveIndex = startIndex;

        // Limit the number of parallel requests
        while (startIndex < m_nodes.count() && startCount < NumOfParallelStart)
        {
            in_addr nodeAddr;
            nodeAddr.s_addr = m_nodes[startIndex]->ipAddress;

            NOTICE("nodes[" << startIndex << "] = " << inet_ntoa(nodeAddr) <<
                   ":" << m_nodes[startIndex]->udpPort << ":" << m_nodes[startIndex]->coreId);

            // Construct packet to send
            hdr->operation = MpiProxy::MpiOpExec;
            hdr->result = 0;
            hdr->rankId = startIndex;
            hdr->coreId = m_nodes[startIndex]->coreId;
            hdr->coreCount = m_nodes.count();

            // Append command-line after the header
            MemoryBlock::copy((char *)packet + sizeof(MpiProxy::Header), *cmdline,
                               sizeof(packet) - sizeof(MpiProxy::Header));

            // Send the packet
            const Result sendResult = sendPacket(startIndex, packet, sizeof(MpiProxy::Header) + cmdline.length());
            if (sendResult != MPI_SUCCESS)
            {
                ERROR("failed to send packet to nodeId " << startIndex << ": result = " << (int) sendResult);
                return sendResult;
            }
            startIndex++;
            startCount++;
        }

        // Wait for acknowledge of each node
        for (Size i = receiveIndex; i < startIndex; i++)
        {
            Size sz;
            sz = sizeof(MpiProxy::Header);

            const Result recvResult = receivePacket(i, MpiProxy::MpiOpExec, &packet, sz);
            if (recvResult != MPI_SUCCESS)
            {
                ERROR("failed to receive acknowledge for MpiOpExec from nodeId " <<
                       i << ": result = " << (int) recvResult);
                return recvResult;
            }
        }
        startCount = 0;
    }

    return MPI_SUCCESS;
}

MpiHost::Result MpiHost::sendPacket(const Size nodeId,
                                    const void *packet,
                                    const Size size) const
{
    DEBUG("nodeId = " << nodeId << " size = " << size);

    const Node *node = m_nodes.get(nodeId);
    if (node == ZERO)
    {
        ERROR("nodeId " << nodeId << " not found");
        return MPI_ERR_ARG;
    }

    // Prepare UDP broadcast datagram
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = node->ipAddress;
    addr.sin_port = htons(node->udpPort);

    // Send the packet
    int result = ::sendto(m_sock, packet, size, 0,
                         (const sockaddr *) &addr, sizeof(addr));
    if (result <= 0)
    {
        ERROR("failed to send UDP datagram: " << strerror(errno));
        return MPI_ERR_IO;
    }

    return MPI_SUCCESS;
}

MpiHost::Result MpiHost::receivePacket(const Size nodeId,
                                       const MpiProxy::Operation operation,
                                       void *packet,
                                       Size & size)
{
    // Lookup the given node
    const Node *node = m_nodes.get(nodeId);
    if (node == ZERO)
    {
        ERROR("nodeId " << nodeId << " not found");
        return MPI_ERR_ARG;
    }

    in_addr nodeAddr;
    nodeAddr.s_addr = node->ipAddress;
    DEBUG("nodeId = " << nodeId << " addr = " << inet_ntoa(nodeAddr) <<
          " operation = " << (int) operation << " size = " << size);

    // Process buffered packets first
    for (ListIterator<Packet *> i(m_packetBuffers[nodeId]); i.hasCurrent(); i++)
    {
        Packet *pkt = i.current();
        const MpiProxy::Header *hdr = (const MpiProxy::Header *) pkt->data;

        if (hdr->operation == operation)
        {
            MemoryBlock::copy(packet, pkt->data, pkt->size);
            size = pkt->size;
            delete[] pkt->data;
            delete pkt;
            i.remove();
            return MPI_SUCCESS;
        }
    }


    // Keep receiving new packets until we have a matching packet
    while (true)
    {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        const Size recvSize = size;

        // Receive UDP datagram
        int r = recvfrom(m_sock, packet, recvSize, 0,
                         (struct sockaddr *) &addr, &len);
        if (r < 0)
        {
            ERROR("failed to receive UDP datagram on socket " << m_sock << ": " << strerror(errno));
            return MPI_ERR_IO;
        }

        const MpiProxy::Header *hdr = (const MpiProxy::Header *) packet;

        DEBUG("received " << r << " bytes from " << inet_ntoa(addr.sin_addr) <<
              ":" << htons(addr.sin_port) << " with coreId = " << hdr->coreId <<
              " rankId = " << hdr->rankId);

        // Is this packet targeted for the given node?
        if (addr.sin_addr.s_addr == node->ipAddress &&
            htons(addr.sin_port) == node->udpPort &&
            hdr->coreId == node->coreId)
        {
            // Verify the MPI operation
            if (hdr->operation != operation)
            {
                ERROR("invalid MPI operation received in packet from node" << nodeId <<
                      " (" << inet_ntoa(nodeAddr) << "): " << (int) hdr->operation <<
                      " != " << (int) operation);
                return MPI_ERR_IO;
            }

            DEBUG("done");
            size = r;
            return MPI_SUCCESS;
        }
        // Add the packet to internal buffers for later retrieval
        else
        {
            Size otherNodeId = 0;

            // Find the corresponding node
            for (Size i = 0; i < m_nodes.count(); i++)
            {
                if (addr.sin_addr.s_addr == m_nodes[i]->ipAddress &&
                    htons(addr.sin_port) == m_nodes[i]->udpPort &&
                    hdr->coreId == m_nodes[i]->coreId)
                {
                    otherNodeId = i;
                    break;
                }
            }

            if (otherNodeId == 0)
            {
                ERROR("nodeId not found for packet from " << inet_ntoa(addr.sin_addr) <<
                      " at port " << htons(addr.sin_port));
            }
            else
            {
                Packet *pkt = new Packet;
                if (!pkt)
                {
                    ERROR("failed to allocate Packet struct for buffering: " << strerror(errno));
                    return MPI_ERR_NO_MEM;
                }

                pkt->data = new u8[r];
                if (!pkt->data)
                {
                    ERROR("failed to allocate memory for buffered packet: " << strerror(errno));
                    return MPI_ERR_NO_MEM;
                }

                MemoryBlock::copy(pkt->data, hdr, r);
                pkt->size = r;
                m_packetBuffers[otherNodeId]->append(pkt);
            }
        }
    }

    return MPI_SUCCESS;
}
