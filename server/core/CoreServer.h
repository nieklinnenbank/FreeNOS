/*
 * Copyright (C) 2009 Niek Linnenbank
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

#ifndef __SERVER_CORE_CORESERVER_H
#define __SERVER_CORE_CORESERVER_H

#include <FreeNOS/System.h>
#include <ChannelServer.h>
#include <List.h>
#include <Types.h>
#include <Macros.h>
#include <Index.h>
#include <ExecutableFormat.h>
#include <MemoryChannel.h>
#include <FileSystemMessage.h>
#include <CoreInfo.h>
#include <CoreManager.h>

/**
 * @addtogroup server
 * @{
 *
 * @addtogroup core
 * @{
 */

/**
 * Represents a single Core in a Central Processing Unit (CPU).
 *
 * Each core in a system will run its own instance of CoreServer.
 * CoreServers will communicate and collaborate together to implement functionality.
 */
class CoreServer : public ChannelServer<CoreServer, FileSystemMessage>
{
  private:

    /** Number of times to busy wait on receiving a message */
    static const Size MaxMessageRetry = 128;

    /** The default kernel for starting new cores. */
    static const char *kernelPath;

  public:

    /**
     * Result codes.
     */
    enum Result
    {
        Success,
        NotFound,
        BootError,
        ExecError,
        OutOfMemory,
        IOError,
        MemoryError
    };

  public:

    /**
     * Class constructor function.
     */
    CoreServer();

    /**
     * Run a ping-pong test
     *
     * @return Result code
     */
    Result test();

    /**
     * Routine for the slave processor core
     *
     * @return Exit code
     */
    int runCore();

    /**
     * Initialize the server
     *
     * @return Result code
     */
    virtual Result initialize();

  private:

    /**
     * Boot a processor core
     *
     * @param coreId Core identifier number
     * @param info CoreInfo pointer containing specific core information
     *
     * @return Result code
     */
    virtual Result bootCore(uint coreId, CoreInfo *info) = 0;

    /**
     * Discover processor cores
     *
     * @return Result code
     */
    virtual Result discoverCores() = 0;

    /**
     * Wait for Inter-Processor-Interrupt
     */
    virtual void waitIPI() const = 0;

    /**
     * Send Inter-Processor-Interrupt
     *
     * @param coreId Core identifier number
     *
     * @return Result code
     */
    virtual Result sendIPI(uint coreId) = 0;

  private:

    /**
     * Prepare processor core for booting
     *
     * @param coreId Core identifier number
     * @param info CoreInfo pointer containing specific core information
     * @param regions Kernel executable memory regions
     *
     * @return Result code
     */
    Result prepareCore(uint coreId, CoreInfo *info, ExecutableFormat::Region *regions);

    /**
     * Prepare the CoreInfo array
     *
     * @return Result code
     */
    Result prepareCoreInfo();

    /**
     * Load operating system kernel
     *
     * @return Result code
     */
    Result loadKernel();

    /**
     * Boot all processor cores
     *
     * @return Result code
     */
    Result bootAll();

    /**
     * Setup communication channels between CoreServers
     *
     * @return Exit code
     */
    Result setupChannels();

    /**
     * Clear memory pages with zeroes
     *
     * @param addr Physical memory address to clear
     * @param size Number of bytes to clear
     *
     * @return Exit code
     */
    Result clearPages(Address addr, Size size);

    /**
     * Get and fill the number of processor cores
     *
     * @param msg FileSystemMessage to fill in the core count
     *
     * @return Exit code
     */
    void getCoreCount(FileSystemMessage *msg);

    /**
     * Create a process on the current processor core
     *
     * @param msg FileSystemMessage containing process information
     *
     * @return Exit code
     */
    void createProcess(FileSystemMessage *msg);

    /**
     * Receive message from master
     *
     * @param msg FileSystemMessage pointer
     *
     * @return Result code
     */
    Result receiveFromMaster(FileSystemMessage *msg);

    /**
     * Send message to master
     *
     * @param msg FileSystemMessage pointer
     *
     * @return Result code
     */
    Result sendToMaster(FileSystemMessage *msg);

    /**
     * Receive message from slave
     *
     * @param coreId Core identifier
     * @param msg FileSystemMessage pointer
     *
     * @return Result code
     */
    Result receiveFromSlave(uint coreId, FileSystemMessage *msg);

    /**
     * Send message to slave
     *
     * @param coreId Core identifier
     * @param msg FileSystemMessage pointer
     *
     * @return Result code
     */
    Result sendToSlave(uint coreId, FileSystemMessage *msg);

  protected:

    CoreManager *m_cores;

  private:

    ExecutableFormat *m_kernel;
    u8 *m_kernelImage;

    ExecutableFormat::Region m_regions[16];

    Size m_numRegions;

    Index<CoreInfo> *m_coreInfo;
    SystemInformation m_info;

    Index<MemoryChannel> *m_fromSlave;
    Index<MemoryChannel> *m_toSlave;

    MemoryChannel *m_toMaster;
    MemoryChannel *m_fromMaster;
};

/**
 * @}
 * @}
 */

#endif /* __SERVER_CORE_CORESERVER_H */
