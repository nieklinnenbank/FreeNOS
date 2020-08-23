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

#ifndef __LIB_LIBARCH_HOST_HOSTSHARES_H
#define __LIB_LIBARCH_HOST_HOSTSHARES_H

#include <FreeNOS/User.h>
#include <Types.h>
#include <HashTable.h>
#include <Singleton.h>
#include <MemoryChannel.h>

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libarch
 * @{
 *
 * @addtogroup libarch_host
 * @{
 */

/**
 * Emulates VMShare() on the host OS using POSIX shared memory
 */
class HostShareManager : public StrictSingleton<HostShareManager>
{
  private:

    /** Maximum number of retries when sending a signal/message. */
    static const Size MaximumRetries = 60u;

    /** Wait time in microseconds between each retry. */
    static const Size RetryWaitTimeUs = 100000;

    /** Use the same result codes as VMShare(). */
    typedef API::Result Result;

  public:

    /**
     * Constructor
     */
    HostShareManager();

    /**
     * Destructor
     */
    ~HostShareManager();

    /**
     * Start the manager.
     *
     * @return Result code
     */
    Result initialize();

    /**
     * Stops the manager.
     *
     * @return Result code
     */
    Result terminate();

    /**
     * Create a new share
     *
     * @param pid Target process identifier
     *
     * @return Result code
     */
    Result createShare(const ProcessID pid,
                       ProcessShares::MemoryShare *share,
                       const bool initialize = true,
                       const bool notify = true);

    /**
     * Read an existing share
     *
     * @param pid Target process identifier
     *
     * @return Result code
     */
    Result readShare(const ProcessID pid, ProcessShares::MemoryShare *share);

    /**
     * Remove shares for a Process.
     *
     * @param pid Target process identifier
     *
     * @return Result code
     */
    Result deleteShares(const ProcessID pid);

    /**
     * Called whenever another process created a share for IPC with us.
     *
     * @param pid ProcessID of the remote process that created the share.
     */
    void notifyShareCreated(const ProcessID pid);

  private:

    /**
     * Retrieve unique name for channel between target process and this process.
     *
     * @param pid Target process
     * @param buf Output buffer for the name
     * @Param size Size of the buffer in bytes
     */
    void getChannelName(const ProcessID pid,
                        char *buf,
                        const Size size) const;

    /**
     * Marks this process as initialized.
     *
     * This is required in order to guarantee this process can properly
     * receive signals from other processes (e.g. SIGUSR1).
     *
     * @param ready True to mark ready, false otherwise
     */
    void setReady(const bool ready) const;

    /**
     * Check if another process is initialized
     *
     * @param pid Process identifier
     *
     * @return True if initialized, false otherwise
     */
    bool isReady(const ProcessID pid) const;

  private:

    /** Keeps track of all known shares. */
    HashTable<ProcessID, ProcessShares::MemoryShare *> m_shares;

    /** Kernel event channel producer. */
    MemoryChannel m_kernelChannel;
};

/**
 * @}
 * @}
 * @}
 */

#endif /* __LIB_LIBARCH_HOST_HOSTSHARES_H */
