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
#include <ExecutableFormat.h>
#include <Lz4Decompressor.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include "CoreServer.h"

const char * CoreServer::kernelPath = "/boot/kernel";

CoreServer::CoreServer()
    : ChannelServer<CoreServer, CoreMessage>(this)
{
    m_numRegions = 0;
    m_kernel = ZERO;
    MemoryBlock::set(&m_kernelImage, 0, sizeof(m_kernelImage));
    m_coreInfo = ZERO;

    m_cores = ZERO;
    m_toMaster = ZERO;
    m_fromMaster = ZERO;
    m_toSlave = ZERO;
    m_fromSlave = ZERO;

    // Register IPC handlers
    addIPCHandler(Core::GetCoreCount,  &CoreServer::getCoreCount);

    // Because of waitpid() we must send the reply manually before waitpid().
    addIPCHandler(Core::CreateProcess, &CoreServer::createProcess, false);
}

int CoreServer::runCore()
{
    CoreMessage msg;

    if (m_info.coreId == 0)
        return run();

    while (true)
    {
        // wait from a message of the master core
        receiveFromMaster(&msg);

        const MessageHandler<IPCHandlerFunction> *h = m_ipcHandlers.get(msg.action);
        if (h)
        {
            const bool sendReply = h->sendReply;
            (this->*h->exec) (&msg);

            if (sendReply)
            {
                sendToMaster(&msg);
            }
        }
        else
        {
            ERROR("invalid action " << (int)msg.action << " from master");
        }
    }
}

void CoreServer::createProcess(CoreMessage *msg)
{
    const Size maximumArguments = 64;
    char cmd[128], *argv[maximumArguments], *arg = ZERO;
    Memory::Range range;
    API::Result result = API::Success;
    Size argc = 0;

    if (m_info.coreId == 0)
    {
        // Find physical address for program buffer
        range.virt = msg->programAddr;
        if ((result = VMCtl(msg->from, LookupVirtual, &range)) != API::Success)
        {
            ERROR("failed to lookup virtual address at " <<
                  (void *) msg->programAddr << ": " << (int)result);
            msg->result = Core::InvalidArgument;
            return;
        }
        msg->programAddr = range.phys;

        // Find physical address for command
        range.virt = (Address) msg->programCmd;
        if ((result = VMCtl(msg->from, LookupVirtual, &range)) != API::Success)
        {
            ERROR("failed to lookup virtual address at " <<
                  (void *) msg->programCmd << ": " << (int)result);
            msg->result = Core::InvalidArgument;
            return;
        }
        msg->programCmd = (char *) range.phys;

        // Forward message to slave core
        if (sendToSlave(msg->coreNumber, msg) != Core::Success)
        {
            ERROR("failed to write channel on core"<<msg->coreNumber);
            msg->result = Core::IOError;
            return;
        }
        DEBUG("creating program at phys " << (void *) msg->programAddr << " on core" << msg->coreNumber);

        // Wait until the slave created the program
        if (receiveFromSlave(msg->coreNumber, msg) != Core::Success)
        {
            ERROR("failed to read channel on core" << msg->coreNumber);
            msg->result = Core::IOError;
            return;
        }
        DEBUG("program created with result " << (int)msg->result << " at core" << msg->coreNumber);
        ChannelClient::instance()->syncSendTo(msg, sizeof(*msg), msg->from);
    }
    else
    {
        // Copy the program command
        result = VMCopy(SELF, API::ReadPhys, (Address) cmd,
                       (Address) msg->programCmd, sizeof(cmd));
        if (result != API::Success)
        {
            ERROR("failed to copy program command: result = " << (int) result);
            msg->result = Core::InvalidArgument;
            sendToMaster(msg);
            return;
        }
        // First argument points to start of command
        arg = cmd;

        // Translate space separated command to argv[]
        for (Size i = 0; i < sizeof(cmd) && argc < maximumArguments - 1; i++)
        {
            if (cmd[i] == ' ')
            {
                cmd[i] = 0;
                argv[argc++] = arg;
                arg = &cmd[i+1];
            }
            else if (cmd[i] == 0)
            {
                argv[argc++] = arg;
                break;
            }
        }
        // Mark end of the argument list
        argv[argc] = 0;

        // Map the program buffer
        range.phys   = msg->programAddr;
        range.virt   = 0;
        range.access = Memory::Readable | Memory::User;
        range.size   = msg->programSize;
        if ((result = VMCtl(SELF, MapContiguous, &range)) != API::Success)
        {
            ERROR("failed to map program data: " << (int)result);
            msg->result = Core::IOError;
            sendToMaster(msg);
            return;
        }

        int pid = spawn(range.virt, msg->programSize, (const char **)argv);
        if (pid == -1)
        {
            ERROR("failed to spawn() program: " << pid);
            msg->result = Core::IOError;
            sendToMaster(msg);
        }
        else
        {
            // reply to master before calling waitpid()
            msg->result = Core::Success;
            sendToMaster(msg);
        }

        if ((result = VMCtl(SELF, UnMap, &range)) != API::Success)
        {
            ERROR("failed to unmap program data: " << (int)result);
        }

        // Wait until the spawned process completes
        if (pid != -1)
        {
            int status;
            waitpid((pid_t)pid, &status, 0);
        }
    }
}

void CoreServer::getCoreCount(CoreMessage *msg)
{
    DEBUG("");

    if (m_info.coreId == 0)
    {
        if (m_cores)
            msg->coreNumber = m_cores->getCores().count();
        else
            msg->coreNumber = 1;

        msg->result = Core::Success;
    }
    else
        msg->result = Core::InvalidArgument;
}

Core::Result CoreServer::test()
{
    const Size pingPongNumber = 0x12345678;

    if (m_info.coreId != 0)
    {
        CoreMessage msg;
        msg.type   = ChannelMessage::Response;
        msg.action = Core::PongResponse;
        msg.coreNumber = pingPongNumber;

        sendToMaster(&msg);
    }
    else if (m_cores != NULL)
    {
        CoreMessage msg;
        Size numCores = m_cores->getCores().count();

        for (Size i = 1; i < numCores; i++)
        {
            receiveFromSlave(i, &msg);

            if (msg.action == Core::PongResponse && msg.coreNumber == pingPongNumber)
            {
                NOTICE("core" << i << " send a Pong");
            }
            else
            {
                ERROR("invalid message received from core" << i);
            }
        }
    }

    return Core::Success;
}

CoreServer::Result CoreServer::initialize()
{
    // Only core0 needs to start other coreservers
    if (m_info.coreId != 0)
    {
        if (setupChannels() != Core::Success)
        {
            ERROR("failed to setup IPC channels");
            return IOError;
        }
        else
        {
            return Success;
        }
    }

    if (loadKernel() != Core::Success)
    {
        ERROR("failed to load kernel program");
        return IOError;
    }

    if (discoverCores() != Core::Success)
    {
        ERROR("failed to discover cores");
        return IOError;
    }

    if (prepareCoreInfo() != Core::Success)
    {
        ERROR("failed to prepare CoreInfo data array");
        return IOError;
    }

    if (setupChannels() != Core::Success)
    {
        ERROR("failed to setup IPC channels");
        return IOError;
    }

    if (bootAll() != Core::Success)
    {
        ERROR("failed to boot all cores");
        return IOError;
    }

    if (unloadKernel() != Core::Success)
    {
        ERROR("failed to unload kernel program");
        return IOError;
    }

    return Success;
}

Core::Result CoreServer::loadKernel()
{
    struct stat st;
    int fd, r;
    API::Result result;

    DEBUG("Opening : " << kernelPath);

    // Stat the compressed program image
    if ((r = stat(kernelPath, &st)) != 0)
    {
        ERROR("failed to stat() kernel on path: " << kernelPath <<
              ": result " << r);
        return Core::IOError;
    }

    // Map memory buffer for the compressed program image
    Memory::Range compressed;
    compressed.virt   = ZERO;
    compressed.phys   = ZERO;
    compressed.size   = st.st_size;
    compressed.access = Memory::User|Memory::Readable|Memory::Writable;

    // Allocate compressed memory buffer
    result = VMCtl(SELF, MapContiguous, &compressed);
    if (result != API::Success)
    {
        ERROR("failed to allocate compressed kernel image with VMCtl: result = " << (int) result);
        return Core::IOError;
    }

    // Open the file
    if ((fd = open(kernelPath, O_RDONLY)) < 0)
    {
        ERROR("failed to open() kernel on path: " << kernelPath <<
              ": result " << fd);
        return Core::IOError;
    }

    // Read the file
    if ((r = read(fd, (void *) compressed.virt, st.st_size)) != st.st_size)
    {
        ERROR("failed to read() kernel on path: " << kernelPath <<
              ": result " << r);
        return Core::IOError;
    }
    close(fd);

    // Initialize decompressor
    Lz4Decompressor lz4((void *)compressed.virt, st.st_size);
    Lz4Decompressor::Result decompResult = lz4.initialize();
    if (decompResult != Lz4Decompressor::Success)
    {
        ERROR("failed to initialize LZ4 decompressor: result = " << (int) decompResult);
        return Core::IOError;
    }

    // Map memory buffer for the uncompressed program image
    m_kernelImage.virt   = ZERO;
    m_kernelImage.phys   = ZERO;
    m_kernelImage.size   = lz4.getUncompressedSize();
    m_kernelImage.access = Memory::User|Memory::Readable|Memory::Writable;

    // Allocate uncompressed memory buffer
    result = VMCtl(SELF, MapContiguous, &m_kernelImage);
    if (result != API::Success)
    {
        ERROR("failed to allocate kernel image with VMCtl: result = " << (int) result);
        return Core::IOError;
    }

    // Decompress the kernel program
    decompResult = lz4.read((void *)m_kernelImage.virt, lz4.getUncompressedSize());
    if (decompResult != Lz4Decompressor::Success)
    {
        ERROR("failed to decompress kernel image: result = " << (int) decompResult);
        return Core::IOError;
    }

    // Attempt to read executable format
    ExecutableFormat::Result execResult = ExecutableFormat::find((const u8 *)m_kernelImage.virt, st.st_size, &m_kernel);
    if (execResult != ExecutableFormat::Success)
    {
        ERROR("failed to find ExecutableFormat of kernel on path: " << kernelPath <<
             ": result " << (int) execResult);
        return Core::ExecError;
    }

    // Retrieve memory regions
    m_numRegions = 16;
    execResult = m_kernel->regions(m_regions, &m_numRegions);

    if (execResult != ExecutableFormat::Success)
    {
        ERROR("failed to get ExecutableFormat regions of kernel on path: " << kernelPath <<
              ": result " << (int) execResult);
        return Core::ExecError;
    }

    // Release compressed kernel image
    result = VMCtl(SELF, Release, &compressed);
    if (result != API::Success)
    {
        ERROR("failed to release compressed kernel image with VMCtl: result = " << (int) result);
        return Core::IOError;
    }

    DEBUG("kernel loaded");
    return Core::Success;
}

Core::Result CoreServer::unloadKernel()
{
    // Cleanup program buffer
    const API::Result r = VMCtl(SELF, Release, &m_kernelImage);
    if (r != API::Success)
    {
        ERROR("failed to deallocate kernel image with VMCtl: result = " << (int) r);
        return Core::IOError;
    }

    MemoryBlock::set(&m_kernelImage, 0, sizeof(m_kernelImage));
    MemoryBlock::set(&m_regions, 0, sizeof(m_regions));
    return Core::Success;
}

Core::Result CoreServer::prepareCore(uint coreId, CoreInfo *info,
                                           ExecutableFormat::Region *regions)
{
    API::Result r;
    SystemInformation sysInfo;

    DEBUG("Reserving: " << (void *)info->memory.phys << " size=" <<
            info->memory.size << " available=" << sysInfo.memoryAvail);

    // Claim the core's memory
    if ((r = VMCtl(SELF, ReserveMem, &info->memory)) != API::Success)
    {
        ERROR("VMCtl(ReserveMem) failed for core" << coreId <<
              " at " << (void *)info->memory.phys << ": result " << (int) r);
        return Core::OutOfMemory;
    }

    DEBUG("Starting core" << coreId << " with "
          << info->memory.size / 1024 / 1024 << "MB");

    // Map the kernel
    for (Size i = 0; i < m_numRegions; i++)
    {
        Memory::Range range;
        range.phys = info->memory.phys + (regions[i].virt - RAM_ADDR);
        range.virt = 0;
        range.size = regions[i].dataSize;
        range.access = Memory::Readable | Memory::Writable |
                       Memory::User;

        // Map the target kernel's memory for regions[i].size
        if ((r = VMCtl(SELF, MapContiguous, &range)) != 0)
        {
            ERROR("VMCtl(Map) failed for kernel on core" << coreId <<
                  " at " << (void *)range.phys << ": result " << (int) r);
            return Core::OutOfMemory;
        }

        // Copy the kernel to the target core's memory
        r = VMCopy(SELF, API::Write, m_kernelImage.virt + regions[i].dataOffset,
                   range.virt, regions[i].dataSize);
        if (r != API::Success)
        {
            ERROR("VMCopy failed for kernel regions[" << i << "].dataOffset" <<
                  " at " << (void *)regions[i].dataOffset << ": result " << (int) r);
            return Core::MemoryError;
        }

        // Unmap the target kernel's memory
        if ((r = VMCtl(SELF, UnMap, &range)) != API::Success)
        {
            ERROR("VMCtl(UnMap) failed for kernel on core" << coreId <<
                  " at " << (void *)range.phys << ": result " << (int) r);
            return Core::MemoryError;
        }

        DEBUG(kernelPath << "[" << i << "] = " << (void *) m_regions[i].virt <<
              " @ " << (void *) range.phys);
    }

    // Copy the BootImage after the kernel.
    Memory::Range range;
    range.phys = info->bootImageAddress;
    range.virt = 0;
    range.size = info->bootImageSize;
    range.access = Memory::Readable | Memory::Writable | Memory::User;

    // Map BootImage buffer
    if ((r = VMCtl(SELF, MapContiguous, &range)) != API::Success)
    {
        ERROR("VMCtl(Map) failed for BootImage on core" << coreId <<
              " at " << (void *)range.phys << ": result " << (int) r);
        return Core::OutOfMemory;
    }

    // Copy the BootImage
    r = VMCopy(SELF, API::Write, sysInfo.bootImageAddress,
               range.virt, sysInfo.bootImageSize);
    if (r != API::Success)
    {
        ERROR("VMCopy failed for BootIage on core" << coreId <<
              " at " << (void *)sysInfo.bootImageAddress <<
              ": result " << (int) r);
        return Core::MemoryError;
    }

    // Unmap the BootImage
    if ((r = VMCtl(SELF, UnMap, &range)) != API::Success)
    {
        ERROR("VMCtl(UnMap) failed for BootImage on core" << coreId <<
              " at " << (void *)range.phys << ": result " << (int) r);
        return Core::MemoryError;
    }

    return Core::Success;
}


Core::Result CoreServer::prepareCoreInfo()
{
    SystemInformation sysInfo;
    Size memPerCore = 0;

    List<uint> & cores = m_cores->getCores();
    if (cores.count() == 0)
    {
        ERROR("no cores found");
        return Core::NotFound;
    }

    memPerCore = sysInfo.memorySize / cores.count();
    memPerCore /= MegaByte(4);
    memPerCore *= MegaByte(4);

    NOTICE("found " << cores.count() << " cores: " <<
            (memPerCore / 1024 / 1024) << "MB per core");

    // Allocate CoreInfo for each core
    m_coreInfo = new Index<CoreInfo, MaxCores>();

    // Prepare CoreInfo for each core
    for (ListIterator<uint> i(cores); i.hasCurrent(); i++)
    {
        uint coreId = i.current();

        if (coreId != 0)
        {
            CoreInfo *info = new CoreInfo;
            m_coreInfo->insertAt(coreId, info);
            MemoryBlock::set(info, 0, sizeof(CoreInfo));

            info->coreId = coreId;
            info->memory.phys = RAM_ADDR + (memPerCore * coreId);
            info->memory.size = memPerCore - PAGESIZE;
            info->kernel.phys = sysInfo.kernelAddress + (memPerCore * coreId);
            info->kernel.size = sysInfo.kernelSize;

            info->bootImageAddress = info->kernel.phys + info->kernel.size;
            info->bootImageAddress += PAGESIZE - (info->kernel.size % PAGESIZE);
            info->bootImageSize = sysInfo.bootImageSize;

            info->heapAddress = info->bootImageAddress + info->bootImageSize;
            info->heapAddress += PAGESIZE - (info->bootImageSize % PAGESIZE);
            info->heapSize = MegaByte(1);

            info->coreChannelAddress = info->heapAddress + info->heapSize;
            info->coreChannelAddress += PAGESIZE - (info->heapSize % PAGESIZE);
            info->coreChannelSize    = PAGESIZE * 4;
            clearPages(info->coreChannelAddress, info->coreChannelSize);

            m_kernel->entry(&info->kernelEntry);
            info->timerCounter = sysInfo.timerCounter;
            strlcpy(info->kernelCommand, kernelPath, KERNEL_PATHLEN);
        }
    }

    return Core::Success;
}

Core::Result CoreServer::bootAll()
{
    List<uint> & cores = m_cores->getCores();
    if (cores.count() == 0)
    {
        ERROR("no cores found");
        return Core::NotFound;
    }

    // Boot each core
    for (ListIterator<uint> i(cores); i.hasCurrent(); i++)
    {
        uint coreId = i.current();

        if (coreId != 0)
        {
            prepareCore(coreId, (CoreInfo *)m_coreInfo->get(coreId), m_regions);
            bootCore(coreId, (CoreInfo *)m_coreInfo->get(coreId));
        }
    }

    return Core::Success;
}

Core::Result CoreServer::clearPages(Address addr, Size size)
{
    Memory::Range range;

    DEBUG("addr = " << (void*)addr << ", size = " << size);

    range.phys = addr;
    range.virt = ZERO;
    range.size = size;
    range.access = Memory::User | Memory::Readable | Memory::Writable;
    VMCtl(SELF, MapContiguous, &range);

    MemoryBlock::set((void *) range.virt, 0, size);

    VMCtl(SELF, UnMap, &range);
    return Core::Success;
}

Core::Result CoreServer::setupChannels()
{
    SystemInformation info;

    DEBUG("");

    if (info.coreId == 0)
    {
        Size numCores = m_cores->getCores().count();

        m_toSlave    = new Index<MemoryChannel, MaxCores>();
        m_fromSlave  = new Index<MemoryChannel, MaxCores>();

        for (Size i = 1; i < numCores; i++)
        {
            MemoryChannel *ch = new MemoryChannel(Channel::Producer, sizeof(CoreMessage));
            CoreInfo *coreInfo = m_coreInfo->get(i);
            ch->setPhysical(coreInfo->coreChannelAddress + (PAGESIZE * 2),
                            coreInfo->coreChannelAddress + (PAGESIZE * 3));
            m_toSlave->insertAt(i, ch);

            ch = new MemoryChannel(Channel::Consumer, sizeof(CoreMessage));
            ch->setPhysical(coreInfo->coreChannelAddress,
                            coreInfo->coreChannelAddress + PAGESIZE);
            m_fromSlave->insertAt(i, ch);
        }
    }
    else
    {
        m_toMaster = new MemoryChannel(Channel::Producer, sizeof(CoreMessage));
        m_toMaster->setPhysical(info.coreChannelAddress,
                                info.coreChannelAddress + PAGESIZE);

        m_fromMaster = new MemoryChannel(Channel::Consumer, sizeof(CoreMessage));
        m_fromMaster->setPhysical(info.coreChannelAddress + (PAGESIZE * 2),
                                  info.coreChannelAddress + (PAGESIZE * 3));
    }

    return Core::Success;
}

Core::Result CoreServer::receiveFromMaster(CoreMessage *msg)
{
    Channel::Result result = Channel::NotFound;

    // wait from a message of the master core
    while (result != Channel::Success)
    {
        for (uint i = 0; i < MaxMessageRetry && result != Channel::Success; i++)
        {
            result = m_fromMaster->read(msg);
        }

        // Wait for IPI which will wake us
        waitIPI();
    }

    return Core::Success;
}

Core::Result CoreServer::sendToMaster(CoreMessage *msg)
{
    while (m_toMaster->write(msg) != Channel::Success)
        ;

    const MemoryChannel::Result result = m_toMaster->flush();
    if (result != Channel::Success)
    {
        ERROR("failed to flush master channel: result = " << (int) result);
        msg->result = Core::IOError;
        return Core::IOError;
    }

    return Core::Success;
}

Core::Result CoreServer::receiveFromSlave(uint coreId, CoreMessage *msg)
{
    MemoryChannel *ch = m_fromSlave->get(coreId);
    if (!ch)
        return Core::IOError;

    while (ch->read(msg) != Channel::Success)
        ;

    return Core::Success;
}

Core::Result CoreServer::sendToSlave(uint coreId, CoreMessage *msg)
{
    MemoryChannel *ch = m_toSlave->get(coreId);
    if (!ch)
    {
        ERROR("cannot retrieve MemoryChannel for core" << coreId);
        msg->result = Core::NotFound;
        return Core::IOError;
    }

    MemoryChannel::Result result = ch->write(msg);
    if (result != Channel::Success)
    {
        ERROR("failed to write channel on core" << coreId << ": result = " << (int)result);
        msg->result = Core::IOError;
        return Core::IOError;
    }

    result = ch->flush();
    if (result != Channel::Success)
    {
        ERROR("failed to flush channel on core" << coreId << ": result = " << (int)result);
        msg->result = Core::IOError;
        return Core::IOError;
    }

    // Send IPI to ensure the slave wakes up for the message
    if (sendIPI(coreId) != Core::Success)
    {
        ERROR("failed to send IPI to core" << coreId);
        return Core::IOError;
    }

    return Core::Success;
}
