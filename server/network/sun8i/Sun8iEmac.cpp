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

#include <Assert.h>
#include <Log.h>
#include "Sun8iEmac.h"

#pragma clang optimize off
#pragma GCC push_options
#pragma GCC optimize ("O0")

Sun8iEmac::Sun8iEmac(const u32 inode,
                     NetworkServer &server)
    : NetworkDevice(inode, server)
    , m_receiveIndex(0)
{
    DEBUG("");
}

Sun8iEmac::~Sun8iEmac()
{
    DEBUG("");
}

FileSystem::Result Sun8iEmac::initialize()
{
    DEBUG("");

    // Initialize clock subsystem
    const SunxiClockControl::Result ccuResult = m_ccu.initialize();
    if (ccuResult != SunxiClockControl::Success)
    {
        ERROR("failed to initialize Clock Control Unit: result = " << (int) ccuResult);
        return FileSystem::IOError;
    }

    // Initialize system control subsystem
    const SunxiSystemControl::Result sysResult = m_syscon.initialize();
    if (sysResult != SunxiSystemControl::Success)
    {
        ERROR("failed to initialize System Control: result = " << (int) sysResult);
        return FileSystem::IOError;
    }

    // Map hardware registers
    const IO::Result mapResult = m_io.map(MemoryAddress, PAGESIZE,
                                          Memory::User | Memory::Readable |
                                          Memory::Writable | Memory::Device);
    if (mapResult != IO::Success)
    {
        ERROR("failed to map hardware registers: result = " << (int) mapResult);
        return FileSystem::IOError;
    }

    // Allocate receive descriptors
    assert(sizeof(Sun8iEmac::FrameDescriptor) * NetworkQueue::MaxPackets < PAGESIZE);
    m_receiveDescRange.phys = 0;
    m_receiveDescRange.virt = 0;
    m_receiveDescRange.access = Memory::User | Memory::Readable | Memory::Writable | Memory::Device;
    m_receiveDescRange.size = PAGESIZE;

    API::Result vmResult = VMCtl(SELF, MapContiguous, &m_receiveDescRange);
    if (vmResult != API::Success)
    {
        ERROR("failed to allocate receive descriptors: result = " << (int) vmResult);
        return FileSystem::IOError;
    }

    // Allocate transmit descriptors
    m_transmitDescRange.phys = 0;
    m_transmitDescRange.virt = 0;
    m_transmitDescRange.access = Memory::User | Memory::Readable | Memory::Writable | Memory::Device;
    m_transmitDescRange.size = PAGESIZE;

    vmResult = VMCtl(SELF, MapContiguous, &m_transmitDescRange);
    if (vmResult != API::Success)
    {
        ERROR("failed to allocate transmit descriptors: result = " << (int) vmResult);
        return FileSystem::IOError;
    }

    // Reset the hardware
    const FileSystem::Result resetResult = reset();
    if (resetResult != FileSystem::Success)
    {
        ERROR("hardware reset failed: result = " << (int) resetResult);
        return resetResult;
    }

    // Initialize network protocols stack
    const FileSystem::Result result = NetworkDevice::initialize();
    if (result != FileSystem::Success)
    {
        ERROR("failed to initialize NetworkDevice: result = " << (int) result);
        return FileSystem::IOError;
    }

    // Enable interrupts
    const API::Result irqResult = ProcessCtl(SELF, EnableIRQ, InterruptNumber);
    if (irqResult != API::Success)
    {
        ERROR("failed to enable interrupt: result = " << (int) irqResult);
        return FileSystem::IOError;
    }

    return FileSystem::Success;
}

FileSystem::Result Sun8iEmac::getAddress(Ethernet::Address *address)
{
    m_io.read(Sun8iEmac::AddrLow, sizeof(u32), address);
    m_io.read(Sun8iEmac::AddrHigh, sizeof(u16), &address->addr[4]);

    DEBUG("address = " << *address);

    return FileSystem::Success;
}

FileSystem::Result Sun8iEmac::setAddress(const Ethernet::Address *address)
{
    DEBUG("address = " << *address);

    m_io.write(Sun8iEmac::AddrLow, sizeof(u32), address);
    m_io.write(Sun8iEmac::AddrHigh, sizeof(u16), &address->addr[4]);

    return FileSystem::Success;
}

void Sun8iEmac::printRx()
{
    DEBUG("");

    FrameDescriptor *desc = m_receiveDesc[m_receiveIndex];

    DEBUG("receive: current receiveIndex = " << m_receiveIndex << " rx:desc.status = " << (void *)desc->status);
    DEBUG("receive: current desc: = " << (void *) m_io.read(ReceiveCurDesc));
    DEBUG("receive: desc base = " << (void *) m_receiveDescRange.phys << " desc size = " << sizeof(FrameDescriptor));
    DEBUG("receive: DMA status = " << (void *) m_io.read(ReceiveStatus));

    for (Size i = 0; i < m_receiveDesc.count(); i++)
    {
        FrameDescriptor *d = m_receiveDesc[i];

        DEBUG("m_receiveDesc[" << i << "]: status = " << (void *)d->status <<
              ", bufsize = " << d->bufsize << ", bufaddr = " << (void *) d->bufaddr <<
              ", next = " << (void *) d->next)
    }
}

void Sun8iEmac::printTx()
{
    FrameDescriptor *desc = m_transmitDesc[m_transmitIndex];

    DEBUG("transmit: transmitIndex = " << m_transmitIndex << " tx:desc.status = " << (void *)desc->status);
    DEBUG("transmit: current desc: = " << (void *) m_io.read(TransmitCurDesc));
    DEBUG("transmit: desc base = " << (void *) m_transmitDescRange.phys <<
          " desc size = " << sizeof(FrameDescriptor));
    DEBUG("transmit: DMA status = " << (void *) m_io.read(TransmitStatus));
}

FileSystem::Result Sun8iEmac::interrupt(const Size vector)
{
    // Clear interrupt flags by writing back the values read
    const u32 status = m_io.read(IntStatus);
    m_io.write(IntStatus, status);

    // Re-enable interrupts
    m_io.write(IntEnable, 0);
    m_io.write(IntEnable, IntEnableTransmit | IntEnableReceive);

    DEBUG("vector = " << vector << " status = " << (void *) status);
    DEBUG("receivectl1 = " << (void *) m_io.read(ReceiveCtl1) <<
          " transmitctl1 = " << (void *) m_io.read(TransmitCtl1));

    if (status & IntStatusTransmit)
    {
        printTx();

        // Release all packet buffers
        while (m_transmitPackets.count() > 0)
        {
            NetworkQueue::Packet *pkt = m_transmitPackets.pop();
            DEBUG("releasing tx:pkt = " << (void *) pkt);
            m_transmit.release(pkt);
        }
    }

    // Check if packets are received
    const FileSystem::Result result = receive();
    if (result != FileSystem::Success)
    {
        ERROR("failed to receive packets");
    }

    // Re-enable the interrupt line on the interrupt controller
    ProcessCtl(SELF, EnableIRQ, InterruptNumber);
    return FileSystem::Success;
}

FileSystem::Result Sun8iEmac::transmit(NetworkQueue::Packet *pkt)
{
    DEBUG("size = " << pkt->size);

    if (m_transmitPending.count() == NetworkQueue::MaxPackets)
    {
        ERROR("transmit queue full");
        return FileSystem::IOError;
    }

    m_transmitPending.push(pkt);
    return FileSystem::Success;
}

FileSystem::Result Sun8iEmac::startDMA()
{
    DEBUG("");

    // Are we still transmitting?
    if (m_transmitPackets.count() > 0)
    {
        return FileSystem::Success;
    }

    // Transmitter ready. Prepare the queue with pending packets
    while (m_transmitPending.count() > 0)
    {
        Memory::Range range;

        NetworkQueue::Packet *pkt = m_transmitPending.pop();
        range.virt = (Address) pkt->data;
        range.size = PAGESIZE;

        // Retrieve physical address of packet payload memory
        const API::Result vmResult = VMCtl(SELF, LookupVirtual, &range);
        if (vmResult != API::Success)
        {
            ERROR("failed to lookup packet physical address: result = " << (int) vmResult);
            return FileSystem::IOError;
        }

        // Clean cache for packet payload memory
        const API::Result ccResult = VMCtl(SELF, CacheClean, &range);
        if (ccResult != API::Success)
        {
            ERROR("failed to clean data cache at " << (void *) range.virt <<
                  ": result = " << (int) ccResult);
            return FileSystem::IOError;
        }

        m_transmitPackets.push(pkt);

        FrameDescriptor *desc = m_transmitDesc[m_transmitIndex];
        desc->bufsize = pkt->size | (TransmitDescFirst | TransmitDescLast |
                                     TransmitDescRaiseInt | TransmitDescChained);
        desc->bufaddr = range.phys;
        desc->status = FrameDescriptorCtl;
        dsb();

        DEBUG("tx:index = " << m_transmitIndex << " tx:desc.status = " << (void *) desc->status <<
              " tx:size = " << pkt->size << " tx:payload = " << *pkt);

        m_transmitIndex = (m_transmitIndex + 1) % NetworkQueue::MaxPackets;
    }

    // Start transmitter DMA engine
    if (m_transmitPackets.count() > 0)
    {
        printTx();
        m_io.set(TransmitCtl1, TransmitCtl1DmaStart | TransmitCtl1DmaEnable);
    }

    // Done
    return FileSystem::Success;
}

FileSystem::Result Sun8iEmac::receive()
{
    DEBUG("");

    printRx();

    while (true)
    {
        FrameDescriptor *desc = m_receiveDesc[m_receiveIndex];
        assert(desc);

        if (desc->status & FrameDescriptorCtl)
        {
            break;
        }

        NetworkQueue::Packet *pkt = m_receivePackets[m_receiveIndex];
        assert(pkt != ZERO);

        if (!(desc->status & ReceiveDescLast))
        {
            ERROR("last flag not set: skipping packet");
            continue;
        }

        const Size bytes = (desc->status >> ReceiveDescFrmShift) & ReceiveDescFrmMask;
        DEBUG("packet: index = " << m_receiveIndex << " bytes = " << bytes);

        // invalidate cache lines here for the payload
        Memory::Range range;
        range.virt = (Address) pkt->data;
        range.size = PAGESIZE;
        assert(pkt->size <= PAGESIZE);

        const API::Result result = VMCtl(SELF, CacheInvalidate, &range);
        if (result != API::Success)
        {
            ERROR("failed to invalidate cache lines for packet: result = " << (int) result);
            return FileSystem::IOError;
        }

        // Process the packet with networking protocols.
        // Note that we need to remove the Ethernet Frame Check Sequence (FCS)
        // which is a 4-bytes field padded at the end of each packet.
        pkt->size = bytes - sizeof(u32);
        process(pkt);

        // Return descriptor back to the device
        desc->status = FrameDescriptorCtl;
        dsb();

        // Move to the next descriptor
        m_receiveIndex = (m_receiveIndex + 1) % NetworkQueue::MaxPackets;
    }

    return FileSystem::Success;
}

bool Sun8iEmac::miiBusyWait() const
{
    DEBUG("");

    for (Size i = 0; i < MaximumMiiPoll; i++)
    {
        const u32 value = m_io.read(MiiCmd);

        if (!(value & MiiCmdPhyBusy))
        {
            break;
        }
    }

    if (m_io.read(MiiCmd) & MiiCmdPhyBusy)
    {
        ERROR("PHY is busy");
        return false;
    }
    else
    {
        return true;
    }
}

u32 Sun8iEmac::miiRead(const u8 phyAddr,
                       const u8 regAddr)
{
    m_io.write(MiiCmd, ((phyAddr << MiiCmdPhyAddrShift) & MiiCmdPhyAddrMask) |
                       ((regAddr << MiiCmdPhyRegShift) & MiiCmdPhyRegMask) |
                        (MiiCmdPhyCsrDiv128 << MiiCmdPhyCsrShift) |
                         MiiCmdPhyBusy);

    if (!miiBusyWait())
        return 0;

    const u32 data = m_io.read(MiiData);

    DEBUG("phyAddr = " << phyAddr << " regAddr = " << regAddr << " data = " << (void *) data);
    return data;
}

void Sun8iEmac::miiWrite(const u8 phyAddr,
                         const u8 regAddr,
                         const u32 data)
{
    DEBUG("phyAddr = " << phyAddr << " regAddr = " << regAddr << " data = " << (void *) data);

    m_io.write(MiiData, data);
    m_io.write(MiiCmd, ((phyAddr << MiiCmdPhyAddrShift) & MiiCmdPhyAddrMask) |
                       ((regAddr << MiiCmdPhyRegShift) & MiiCmdPhyRegMask) |
                        (MiiCmdPhyCsrDiv128 << MiiCmdPhyCsrShift) |
                         MiiCmdPhyWrite | MiiCmdPhyBusy);

    miiBusyWait();
}

FileSystem::Result Sun8iEmac::resetPhy()
{
    DEBUG("");

    // Enable TX clock
    SunxiClockControl::Result ccuResult = m_ccu.enable(SunxiClockControl::ClockEmacTx);
    if (ccuResult != SunxiClockControl::Success)
    {
        ERROR("failed to enable TX clock: result = " << (int) ccuResult);
        return FileSystem::IOError;
    }

    // De-assert TX Reset
    ccuResult = m_ccu.deassert(SunxiClockControl::ResetEmacTx);
    if (ccuResult != SunxiClockControl::Success)
    {
        ERROR("failed to de-assert TX clock reset: result = " << (int) ccuResult);
        return FileSystem::IOError;
    }

    // Enable Ephy Clock
    ccuResult = m_ccu.enable(SunxiClockControl::ClockEphy);
    if (ccuResult != SunxiClockControl::Success)
    {
        ERROR("failed to enable Ephy clock: result = " << (int) ccuResult);
        return FileSystem::IOError;
    }

    // De-assert Ephy Reset
    ccuResult = m_ccu.deassert(SunxiClockControl::ResetEphy);
    if (ccuResult != SunxiClockControl::Success)
    {
        ERROR("failed to de-assert Ephy clock reset: result = " << (int) ccuResult);
        return FileSystem::IOError;
    }

    // Write system control register to prepare the PHY
    m_syscon.setupEmac(PhyMdioAddress);

    // Read PHY identification
    const u32 idLow = miiRead(PhyMdioAddress, MiiRegIdLow);
    const u32 idHigh = miiRead(PhyMdioAddress, MiiRegIdHigh);
    if (idLow != 0x1400 || idHigh != 0x44)
    {
        WARNING("unrecognized PHY identification: high = " << (void *) idHigh <<
                " low = " << (void *) idLow);
    }
    DEBUG("idHigh = " << (void *)idHigh << " idLow " << (void *)idLow);

    // Reset the PHY via MII
    miiWrite(PhyMdioAddress, MiiRegControl, (1 << 15));

    // Check if the reset bit is cleared
    u32 reg = 0;
    for (Size i = 0; i < MaximumMiiPoll; i++)
    {
        reg = miiRead(PhyMdioAddress, MiiRegControl);
        if (!(reg & MiiControlReset))
        {
            break;
        }
    }

    if (reg & MiiControlReset)
    {
        ERROR("failed to reset PHY: reset bit is high: reg = " << (void *) reg);
        return FileSystem::IOError;
    }

    return FileSystem::Success;
}

FileSystem::Result Sun8iEmac::configPhy()
{
    DEBUG("");

    // Fill in advertised capabilities
    u32 val = miiRead(PhyMdioAddress, MiiRegAdv);
    val |= MiiAdvSpeed10Full;
    val |= MiiAdvSpeed100Full;
    miiWrite(PhyMdioAddress, MiiRegAdv, val);

    // Restart auto-negociation
    val = miiRead(PhyMdioAddress, MiiRegControl);
    val |= MiiControlFullDuplex | MiiControlAutoRestart |
           MiiControlAutoEnable | MiiControlSpeed100;
    miiWrite(PhyMdioAddress, MiiRegControl, val);

    // Wait until auto-negociation is complete and link is up
    for (Size i = 0; i < MaximumMiiPoll; i++)
    {
        val = miiRead(PhyMdioAddress, MiiRegStatus);
        if ((val & MiiStatusAutoCompl) &&
            (val & MiiStatusLink))
        {
            break;
        }
    }

    if (!((val & MiiStatusAutoCompl) &&
          (val & MiiStatusLink)))
    {
        WARNING("timeout waiting for auto negociation/link: status = " << (void *) val);
        return FileSystem::Success;
    }

    DEBUG("status = " << (void *) val);
    return FileSystem::Success;
}

FileSystem::Result Sun8iEmac::resetReceive()
{
    DEBUG("");

    FrameDescriptor *desc = (FrameDescriptor *) m_receiveDescRange.virt;
    Address descPhys = m_receiveDescRange.phys;

    // Reconstruct receive descriptor list
    for (Size i = 0; i < NetworkQueue::MaxPackets; i++)
    {
        const bool last = (i == NetworkQueue::MaxPackets - 1);

        NetworkQueue::Packet *pkt = m_receive.get();
        assert(pkt != ZERO);

        // Find physical address of this packet
        Memory::Range range;
        range.virt = (Address) pkt->data;
        range.size = PAGESIZE;
        const API::Result result = VMCtl(SELF, LookupVirtual, &range);
        if (result != API::Success)
        {
            ERROR("failed to lookup physical address of packet: result = " << (int) result);
            return FileSystem::IOError;
        }

        // Prepare receive descriptor
        desc->status  = FrameDescriptorCtl;
        desc->bufsize = m_maximumPacketSize;
        desc->bufaddr = range.phys;
        desc->next    = last ? m_receiveDescRange.phys : descPhys + sizeof(FrameDescriptor);

        // Move to next descriptor
        m_receiveDesc.insertAt(i, desc);
        m_receivePackets.insertAt(i, pkt);
        desc++;
        descPhys += sizeof(FrameDescriptor);
    }

    dsb();

    // Finalize receive administration
    m_receiveIndex = 0;
    m_io.write(ReceiveDescList, m_receiveDescRange.phys);
    m_io.write(ReceiveCtl0, ReceiveCtl0Enable);
    m_io.write(ReceiveCtl1, ReceiveCtl1DmaEnable | ReceiveCtl1DmaStart |
                            ReceiveCtl1ErrorFrame | ReceiveCtl1UnderFrame | ReceiveCtl1FullFrame);

    return FileSystem::Success;
}

FileSystem::Result Sun8iEmac::resetTransmit()
{
    DEBUG("");

    FrameDescriptor *desc = (FrameDescriptor *) m_transmitDescRange.virt;
    Address descPhys = m_transmitDescRange.phys;

    // Reconstruct transmit descriptor list
    for (Size i = 0; i < NetworkQueue::MaxPackets; i++)
    {
        const bool last = (i == NetworkQueue::MaxPackets - 1);

        // Prepare transmit descriptor
        desc->status  = 0;
        desc->bufsize = 0;
        desc->bufaddr = 0;
        desc->next    = last ? m_transmitDescRange.phys : descPhys + sizeof(FrameDescriptor);

        // Move to next descriptor
        m_transmitDesc.insertAt(i, desc);
        desc++;
        descPhys += sizeof(FrameDescriptor);
    }

    // Finalize administration
    m_transmitIndex = 0;
    m_io.write(TransmitDescList, m_transmitDescRange.phys);
    m_io.write(TransmitCtl0, TransmitCtl0Enable);
    m_io.write(TransmitCtl1, TransmitCtl1FullFrame | TransmitCtl1NextFrame);

    return FileSystem::Success;
}

FileSystem::Result Sun8iEmac::reset()
{
    DEBUG("");

    // Save the current MAC address (written by the bootloader)
    Ethernet::Address mac;
    const FileSystem::Result macRead = getAddress(&mac);
    if (macRead != FileSystem::Success)
    {
        ERROR("failed to read MAC address: result = " << (int) macRead);
        return macRead;
    }

    DEBUG("savedMac = " << mac);

    // Reset the PHY and MDIO bus */
    FileSystem::Result result = resetPhy();
    if (result != FileSystem::Success)
    {
        ERROR("failed to reset PHY: result = " << (int) result);
        return result;
    }

    // Configure PHY
    result = configPhy();
    if (result != FileSystem::Success)
    {
        ERROR("failed to reset PHY: result = " << (int) result);
        return result;
    }

    // Initialize hardware
    m_io.write(BasicCtl1, BasicCtl1Reset);

    // Wait for reset to complete
    for (Size i = 0; i < MaximumResetPoll; i++)
    {
        if (!(m_io.read(BasicCtl1) & BasicCtl1Reset))
        {
            break;
        }
    }

    // Did the hardware reset complete?
    if (m_io.read(BasicCtl1) & BasicCtl1Reset)
    {
        ERROR("basic hardware reset failed");
        return FileSystem::IOError;
    }

    // Apply saved MAC address
    const FileSystem::Result macWrite = setAddress(&mac);
    if (macWrite != FileSystem::Success)
    {
        ERROR("failed to write MAC address: result = " << (int) macWrite);
        return macWrite;
    }

    // Clear and enable interrupts
    m_io.write(IntStatus, 0x1FFFFFF);
    m_io.set(IntEnable, IntEnableTransmit | IntEnableReceive);

    // Set DMA burst length
    m_io.write(BasicCtl1, 8 << BasicCtl1BurstShift);

    const FileSystem::Result rxResult = resetReceive();
    if (rxResult != FileSystem::Success)
    {
        ERROR("failed to reset receive control: result = " << (int) rxResult);
        return rxResult;
    }

    const FileSystem::Result txResult = resetTransmit();
    if (txResult != FileSystem::Success)
    {
        ERROR("failed to reset transmit control: result = " << (int) txResult);
        return txResult;
    }

    // Adjust link bits
    u32 val = m_io.read(BasicCtl0);
    val |= BasicCtl0FullDup;
    val &= ~BasicCtl0SpeedMask;
    val |= BasicCtl0Speed100;
    m_io.write(BasicCtl0, val);

    return FileSystem::Success;
}
