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

#ifndef __LIB_LIBMPI_MPIHOST_H
#define __LIB_LIBMPI_MPIHOST_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <Types.h>
#include <Index.h>
#include <List.h>
#include "MpiBackend.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libmpi
 * @{
 */

/**
 * Implements a MPI backend for the host OS which communicates with mpiproxy servers
 */
class MpiHost : public MpiBackend
{
  private:

    /** Maximum number of supported nodes */
    static const Size MaximumNodes = 512;

  private:

    /**
     * Describes a remote CPU node accessible via MPI.
     */
    struct Node
    {
        in_addr_t ipAddress; /**@< IP address of the node */
        u16 udpPort;         /**@< UDP port of the node */
        u32 coreId;          /**@< Local identifier of the core at the node */
    };

    /**
     * Describes data received via UDP
     */
    struct Packet
    {
        u8 *data;            /**@< Payload data */
        Size size;           /**@< Payload size in bytes */
    };

  public:

    /**
     * Constructor
     */
    MpiHost();

    /**
     * Initialize the backend
     *
     * @param argc Argument count pointer
     * @param argv Argument values array pointer
     *
     * @return Result code
     */
    virtual Result initialize(int *argc,
                              char ***argv);

    /**
     * Terminate the backend
     *
     * @return Result code
     */
    virtual Result terminate();

    /**
     * Retrieve communication rank (core id)
     *
     * @param comm Communication reference
     * @param rank Output the communication rank number
     *
     * @return Result code
     */
    virtual Result getCommRank(MPI_Comm comm,
                               int *rank);

    /**
     * Retrieve communication size (total cores)
     *
     * @param comm Communication reference
     * @param size Output the communication size
     *
     * @return Result code
     */
    virtual Result getCommSize(MPI_Comm comm,
                               int *size);

    /**
     * Synchronous send data
     *
     * @param buf Input data buffer
     * @param count Number of data items
     * @param datatype Type of data
     * @param dest Destination to send to (core id)
     * @param tag Optional data identifier to send
     * @param comm Communication reference
     *
     * @return Result code
     */
    virtual Result send(const void *buf,
                        int count,
                        MPI_Datatype datatype,
                        int dest,
                        int tag,
                        MPI_Comm comm);

    /**
     * Synchronous receive data
     *
     * @param buf Output data buffer
     * @param count Number of data items
     * @param datatype Type of data
     * @param source Source to receive data from (core id)
     * @param tag Optional data identifier to receive
     * @param comm Communication reference
     * @param status Output the MPI status
     *
     * @return Result code
     */
    virtual Result receive(void *buf,
                           int count,
                           MPI_Datatype datatype,
                           int source,
                           int tag,
                           MPI_Comm comm,
                           MPI_Status *status);

  private:

    /**
     * Parse the given hosts file
     *
     * @param hostsfile Path to the hosts file to parse
     *
     * @return Result code
     */
    Result parseHostsFile(const char *hostsfile);

    /**
     * Start remote processes
     *
     * @param argc Argument count passed to remote program
     * @param argv Argument values passed to remote program
     *
     * @return Result code
     */
    Result startProcesses(int argc,
                          char **argv);

    /**
     * Send UDP packet to a remote node
     *
     * @param nodeId Identification number of the node
     * @param packet Buffer with data to send
     * @param size Number of bytes to send
     *
     * @return Result code
     */
    Result sendPacket(const Size nodeId,
                      const void *packet,
                      const Size size) const;

    /**
     * Receive UDP packet from remote node
     *
     * @param nodeId Identification number of the node to receive from
     * @param operation Expected MPI operation value of the packet
     * @param packet Payload output
     * @param size Output for number of bytes received
     *
     * @return Result code
     */
    Result receivePacket(const Size nodeId,
                         const MpiProxy::Operation operation,
                         void *packet,
                         Size & size);

  private:

    /** UDP socket for communicating with remote nodes */
    int m_sock;

    /** Contains all known nodes that participate in the computation */
    Index<Node, MaximumNodes> m_nodes;

    /** Buffers incoming packets for later processing */
    Index<List<Packet *>, MaximumNodes> m_packetBuffers;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBMPI_MPIHOST_H */
