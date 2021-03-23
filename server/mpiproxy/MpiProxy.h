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

#ifndef __SERVER_MPIPROXY_MPIPROXY_H
#define __SERVER_MPIPROXY_MPIPROXY_H

#include <Types.h>
#include <Index.h>
#include <Memory.h>
#include <Array.h>
#include <POSIXApplication.h>
#include <sys/socket.h>

class NetworkClient;
class MemoryChannel;

/**
 * @addtogroup server
 * @{
 *
 * @addtogroup mpiproxy
 * @{
 */

/**
 * Server that bridges IP/UDP to local MPI communication channels
 *
 * @warning This server should only be used on a trusted network.
 *          No authentication or encryption supported!
 *
 * @todo This server might be able re-use the MpiTarget class by inheritance or as a member instance
 *
 * @todo MpiProxy protocol currently uses UDP which does not protect against packet loss or corruption.
 *       A simple solution can be implemented by using retransmissions and acknowledge packets
 */
class MpiProxy : public POSIXApplication
{
  private:

    /** Port number for IP/UDP traffic */
    static const u16 UdpPort = 6660;

    /** Timeout in milliseconds to wait for packet receive */
    static const Size ReceiveTimeoutMs = 500;

    /** Maximum number of supported MPI channels */
    static const Size MaximumChannels = 128u;

  public:

    /** Maximum size of packet payload */
    static const Size MaximumPacketSize = 1448;

    /**
     * Encodes various MPI operations
     */
    enum Operation
    {
        MpiOpSend = 0,
        MpiOpRecv,
        MpiOpExec,
        MpiOpTerminate
    };

    /**
     * Packet payload header for MPI messages via IP/UDP
     */
    struct Header
    {
        u8 operation;
        u8 result;
        u16 rankId;
        u16 coreId;

        union
        {
            u16 coreCount;
            u16 datatype;
        };
        u32 datacount;
    };

  public:

    /**
     * Constructor
     *
     * @param argc Argument count
     * @param argv Argument values
     */
    MpiProxy(int argc, char **argv);

    /**
     * Destructor
     */
    virtual ~MpiProxy();

    /**
     * Initialize the server
     *
     * @return Result code
     */
    virtual Result initialize();

    /**
     * Run the server
     *
     * @return Result code
     */
    virtual Result exec();

  private:

    /**
     * Send UDP packet
     *
     * @param packet Payload input
     * @param size Number of bytes to send
     * @param addr The destination IP and port
     *
     * @return Result code
     */
    Result udpSend(const void *packet,
                   const Size size,
                   const struct sockaddr & addr) const;

    /**
     * Send multiple UDP packets
     *
     * @param vec I/O vector with multiple packets
     * @param count Number of entries in the I/O vector
     * @param addr The destination IP and port
     *
     * @return Result code
     */
    Result udpSendMultiple(const struct iovec *vec,
                           const Size count,
                           const struct sockaddr & addr) const;

    /**
     * Receive UDP packet
     *
     * @param packet Payload output
     * @param size Output for number of bytes received
     * @param addr Output for the source IP and port
     *
     * @return Result code
     */
    Result udpReceive(void *packet,
                      Size & size,
                      struct sockaddr & addr) const;

    /**
     * Process incoming packet
     *
     * @param packet Payload input
     * @param size Number of bytes received
     * @param addr Source IP and port of the packet
     *
     * @return Result code
     */
    Result processRequest(const u8 *packet,
                          const Size size,
                          const struct sockaddr & addr);

    /**
     * Process MPI send request
     *
     * @param header Packet header pointer
     * @param packet Full packet input
     * @param size Number of bytes received
     *
     * @return Result code
     */
    Result processSend(const Header *header,
                       const u8 *packet,
                       const Size size);

    /**
     * Process MPI recv request
     *
     * @param header Packet header pointer
     * @param packet Full packet input
     * @param size Number of bytes received
     * @param addr Source IP and port of the packet
     *
     * @return Result code
     */
    Result processRecv(const Header *header,
                       const u8 *packet,
                       const Size size,
                       const struct sockaddr & addr);

    /**
     * Process execute request
     *
     * @param header Packet header pointer
     * @param packet Full packet input
     * @param size Number of bytes received
     * @param addr Source IP and port of the packet
     *
     * @return Result code
     *
     * @todo Check if a computation is already running and reject the request if so
     */
    Result processExec(const Header *header,
                       const u8 *packet,
                       const Size size,
                       const struct sockaddr & addr);

    /**
     * Process MPI terminate request
     *
     * @param header Packet header pointer
     * @param packet Full packet input
     * @param size Number of bytes received
     * @param addr Source IP and port of the packet
     *
     * @return Result code
     */
    Result processTerminate(const Header *header,
                            const u8 *packet,
                            const Size size,
                            const struct sockaddr & addr);

    /**
     * Create communication channels
     *
     * @param rankId Rank identifier
     * @param coreCount Total core count in the computation
     *
     * @return Result code
     */
    Result createChannels(const Size rankId,
                          const Size coreCount);

    /**
     * Start a process on the local processor
     *
     * @param command Command to execute
     * @param rankId Rank identifier
     * @param coreCount Total cores in the computation
     *
     * @return Result code
     */
    Result startLocalProcess(const char *command,
                             const Size rankId,
                             const Size coreCount);

    /**
     * Start a process on a secondary processor
     *
     * @param coreId Core identifier of secondary processor
     * @param command Command to execute
     * @param rankId Rank identifier
     * @param coreCount Total cores in the computation
     *
     * @return Result code
     */
    Result startRemoteProcess(const Size coreId,
                              const char *command,
                              const Size rankId,
                              const Size coreCount);

  private:

    /** IP/UDP socket for external communication */
    int m_sock;

    /** Networking client object */
    NetworkClient *m_client;

    /** Memory base address for local MPI communication */
    Memory::Range m_memChannelBase;

    /** Stores all channels for receiving data from processes */
    Index<MemoryChannel, MaximumChannels> m_readChannels;

    /** Stores all channels for sending data to processes */
    Index<MemoryChannel, MaximumChannels> m_writeChannels;

    /** Records the PID of each process participating in the computation */
    Array<ProcessID, MaximumChannels> m_pids;
};

/**
 * @}
 * @}
 */

#endif /* __SERVER_MPIPROXY_MPIPROXY_H */
