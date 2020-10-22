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

#include <FreeNOS/ProcessEvent.h>
#include <Assert.h>
#include <HashIterator.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "HostShares.h"

static void signal_sharecreated(int sig, siginfo_t *siginfo, void *context)
{
    HostShareManager::instance()->notifyShareCreated(siginfo->si_pid);
}

static void signal_terminate(int sig, siginfo_t *siginfo, void *context)
{
    HostShareManager::instance()->terminate();
    ::exit(EXIT_SUCCESS);
}

HostShareManager::HostShareManager()
    : m_kernelChannel(Channel::Producer, sizeof(ProcessEvent))
{
    initialize();
}

HostShareManager::~HostShareManager()
{
    terminate();
}

HostShareManager::Result HostShareManager::initialize()
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));

    // Handle signal for receiving new shares
    act.sa_sigaction = &signal_sharecreated;
    act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGUSR1, &act, (struct sigaction *)NULL) < 0)
    {
        perror("sigaction");
        ::exit(EXIT_FAILURE);
        return API::IOError;
    }

    // On terminate, we must cleanup all shares
    act.sa_sigaction = &signal_terminate;
    if (sigaction(SIGTERM, &act, (struct sigaction *) NULL) < 0)
    {
        perror("sigaction");
        ::exit(EXIT_FAILURE);
        return API::IOError;
    }

    // Setup the kernel channel for this process
    ProcessShares::MemoryShare share;
    createShare(KERNEL_PID, &share, true, false);
    m_kernelChannel.setVirtual(share.range.virt, share.range.virt + PAGESIZE);

    // Done initializing. We can receive signals now.
    setReady(true);
    return API::Success;
}

HostShareManager::Result HostShareManager::terminate()
{
    char name[1024];

    for (HashIterator<ProcessID, ProcessShares::MemoryShare *> i(m_shares); i.hasCurrent();)
    {
        ProcessShares::MemoryShare *share = i.current();
        getChannelName(share->pid, name, sizeof(name));
        shm_unlink(name);
        delete share;
        i.remove();
    }

    setReady(false);
    return API::Success;
}

HostShareManager::Result HostShareManager::createShare(
    const ProcessID pid,
    ProcessShares::MemoryShare *share,
    const bool initialize,
    const bool notify)
{
    char name[1024];
    const Size sz = PAGESIZE * 4;
    int fd = -1;

    // Format the filename properly
    getChannelName(pid, name, sizeof(name));

    // Open the shared memory file
    fd = shm_open(name, O_CREAT | O_RDWR, 0600);
    if (fd == -1)
    {
        perror("shm_open");
        return API::IOError;
    }

    // Re-sizes the file, if needed
    if (initialize)
    {
        if (ftruncate(fd, sz) != 0)
        {
            perror("ftruncate");
            close(fd);
            return API::IOError;
        }
    }

    // Map it directly into memory
    u8 *ptr = (u8 *) mmap(0, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == (u8 *) MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return API::IOError;
    }

    // Initialize to zero, if needed
    if (initialize)
    {
        memset(ptr, 0, sz);
    }

    // Fill the output share structure
    share->pid = pid;
    share->range.virt = (Address) ptr;
    share->range.size = sz;
    share->range.phys = 0;
    share->range.access = Memory::Readable | Memory::Writable | Memory::User;

    // Add copy to internal administration
    ProcessShares::MemoryShare *shareCopy = new ProcessShares::MemoryShare;
    memcpy(shareCopy, share, sizeof(*shareCopy));
    m_shares.insert(pid, shareCopy);

    // Signal the remote process to raise kernel event here, if needed
    if (notify)
    {
        // The other process must be ready to receive this signal
        for (Size i = 0; i < MaximumRetries; i++)
        {
            if (isReady(pid))
                break;
            else
                usleep(RetryWaitTimeUs);
        }

        if (!isReady(pid))
        {
            printf("PID %u is not ready to receive\n", pid);
            return API::IOError;
        }

        int r = kill(pid, SIGUSR1);
        if (r != 0)
        {
            perror("kill");
            munmap(ptr, sz);
            close(fd);
            shm_unlink(name);
            return API::IOError;
        }
    }

    // Success
    return API::Success;
}

HostShareManager::Result HostShareManager::readShare(
    const ProcessID pid,
    ProcessShares::MemoryShare *share)
{
    ProcessShares::MemoryShare * const * sh = m_shares.get(pid);
    if (!sh)
        return API::NotFound;

    memcpy(share, (*sh), sizeof(*share));
    return API::Success;
}

HostShareManager::Result HostShareManager::deleteShares(
    const ProcessID pid)
{
    return API::InvalidArgument;
}

void HostShareManager::notifyShareCreated(const ProcessID pid)
{
    ProcessShares::MemoryShare share;
    const Result result = createShare(pid, &share, false, false);
    if (result != API::Success)
    {
        printf("failed to process ShareCreated\n");
        ::exit(EXIT_FAILURE);
    }

    // Prepare ShareCreated event
    ProcessEvent event;
    event.type   = ShareCreated;
    event.number = pid;
    memcpy(&event.share, &share, sizeof(share));

    // Raise ShareCreated event
    if (m_kernelChannel.write(&event) != MemoryChannel::Success)
    {
        printf("failed to write kernel MemoryChannel\n");
        ::exit(EXIT_FAILURE);
    }
    m_kernelChannel.flush();
}

void HostShareManager::getChannelName(const ProcessID pid,
                                      char *buf,
                                      const Size size) const
{
    const ProcessID own_pid = getpid();

    if (own_pid < pid)
        snprintf(buf, size, "/FreeNOS.%u.%u", own_pid, pid);
    else
        snprintf(buf, size, "/FreeNOS.%u.%u", pid, own_pid);
}

void HostShareManager::setReady(const bool ready) const
{
    char readyFile[32];
    snprintf(readyFile, sizeof(readyFile), "/FreeNOS.%u.ready", getpid());

    if (ready)
    {
        int fd = shm_open(readyFile, O_CREAT | O_RDWR, 0600);
        if (fd == -1)
        {
            perror("shm_open");
            ::exit(EXIT_FAILURE);
        }
    }
    else
    {
        shm_unlink(readyFile);
    }
}

bool HostShareManager::isReady(const ProcessID pid) const
{
    char readyFile[32];

    snprintf(readyFile, sizeof(readyFile), "/FreeNOS.%u.ready", pid);

    int fd = shm_open(readyFile, O_RDWR, 0600);
    if (fd == -1)
    {
        return false;
    }

    close(fd);
    return true;
}
