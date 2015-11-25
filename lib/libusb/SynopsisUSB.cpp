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

#include <unistd.h>
#include "USBDescriptor.h"
#include "SynopsisUSB.h"

SynopsisUSB::SynopsisUSB(const char *path)
    : USBController(path)
{
}

Error SynopsisUSB::initialize()
{
    Error r = USBController::initialize();

    if (r != ESUCCESS)
        return r;

    // Map USB host controller registers
    if (m_io.map(IO_BASE + Base, PAGESIZE*2,
                 Memory::User|Memory::Readable|Memory::Writable|Memory::Device) != IO::Success)
    {
        ERROR("failed to map I/O registers");
        return EIO;
    }

    // Check device ID
    if (m_io.read(VendorId) != DefaultVendorId)
    {
        ERROR("incompatible vendorId: " << m_io.read(VendorId) << " != " << DefaultVendorId);
        return EIO;
    }

    DEBUG("UserId: " << m_io.read(UserId) << " VendorId: " << m_io.read(VendorId));
    NOTICE("Synopsis Design Ware USB-on-the-go Host Controller found");

    // Initialize power manager
    if (m_power.initialize() != BroadcomPower::Success)
    {
        ERROR("failed to initialize power manager");
        return EIO;
    }
    // Power on the USB subsystem
    if (m_power.enable(BroadcomPower::USB) != BroadcomPower::Success)
    {
        ERROR("failed to power on the USB subsystem");
        return EIO;
    }
    DEBUG("powered on");

    // Soft-Reset
    softReset();
    DEBUG("software reset done");

    // Setup DMA
    m_io.write(RxFIFOSize, 1024);
    m_io.write(TxFIFOSize, (1024 << 16) | 1024);
    m_io.write(PeriodTxFIFOSize, (1024 << 16) | 2048);
    m_io.set(AHBConfig, DMAEnable | AXIWait);

    // Enable the USB controller interrupt on the ARM's interrupt controller
    addIRQHandler(InterruptNumber, (IRQHandlerFunction) &SynopsisUSB::interruptHandler);
    addIRQHandler(0, (IRQHandlerFunction) &SynopsisUSB::interruptHandler); // TODO: ARM does not have IRQ_REG() yet
    ProcessCtl(SELF, WatchIRQ, InterruptNumber);
    ProcessCtl(SELF, EnableIRQ, InterruptNumber);
    DEBUG("interrupt handler installed");

    // Clear all pending core interrupts
    m_io.write(CoreIntMask, 0);
    m_io.write(CoreInterrupt, 0xffffffff);

    // Enable core host channel and port interrupts
    m_io.write(CoreIntMask, CoreIntChannel | CoreIntPort);

    // Enable interrupts globally on the USB host controller.
    m_io.set(AHBConfig, InterruptEnable);
    DEBUG("interrupts enabled");

    // Power-on host port (virtual root hub)
    u32 val = m_io.read(HostPortControl);
    val &= ~(HostPortEnable | HostPortEnableChanged |
             HostPortConnectChanged | HostPortCurrentChanged);
    val |= HostPortPower;
    m_io.write(HostPortControl, val);
    DEBUG("poweron host port");

    // Begin host port reset (raise the reset signal)
    val = m_io.read(HostPortControl);
    val &= ~(HostPortEnable | HostPortEnableChanged |
             HostPortConnectChanged | HostPortCurrentChanged);
    val |=  HostPortReset;
    m_io.write(HostPortControl, val);

#warning TODO: implement real sleep() now. We need it here.
    sleep(60);

    // Finish host port reset (lower the reset signal)
    val = m_io.read(HostPortControl);
    val &= ~(HostPortEnable | HostPortEnableChanged |
             HostPortConnectChanged | HostPortCurrentChanged);
    m_io.write(HostPortControl, val);
    DEBUG("host port reset done");

    // Clear host port interrupt flags
    val = m_io.read(HostPortControl);
    val |= HostPortEnableChanged | HostPortConnectChanged | HostPortCurrentChanged;
    m_io.write(HostPortControl, val);

    DEBUG("host port (root hub) enabled");
    DEBUG("host port status=" << m_io.read(HostPortControl) << " connected=" <<
        (m_io.read(HostPortControl) & HostPortConnect));

    // Done.
    return ESUCCESS;
}

void SynopsisUSB::interruptHandler(InterruptMessage *msg)
{
    DEBUG("");

    Address channelInt  = m_io.read(HostChannelInt);

    // Post-process channel interrupt for each channel
    // which has the interrupt flag set high.
    for (Size i = 0; i < ChannelCount; i++)
    {
        if (channelInt & (1 << i))
            channelInterrupt(i);
    }
    return DeviceServer::interruptHandler(msg);
}

#warning perhaps make a SynopsisChannel class? It will simplify this code.

void SynopsisUSB::channelInterrupt(Size channelId)
{
    DEBUG("channelId =" << channelId);
    Address channelBase = HostChannel + (HostChannelSize * channelId);

    // Clear interrupt
    m_io.write(channelBase + ChannelMask, 0);
    m_io.write(channelBase + ChannelInterrupt, 0xffffffff);
}

void SynopsisUSB::softReset()
{
    DEBUG("");

    // Raise reset flag
    m_io.write(CoreReset, 1);

    // Wait until cleared
    while (m_io.read(CoreReset) & 1);
}

void SynopsisUSB::startTransfer(Size channelId,
                                const FileSystemMessage *msg,
                                USBMessage *usb)
{
    DEBUG("");

    Address channelBase = HostChannel + (HostChannelSize * channelId);
    Size size, packetCount, frameNumber;
    PacketId packetId;
    USBTransfer::Direction dir;
    Address data;

    switch (usb->state)
    {
        case USBMessage::Setup:
            size = sizeof(USBTransfer::DeviceSetup);
            packetId = SetupId;
            data = (Address) &usb->setup;
            dir = USBTransfer::Out;
            break;

        case USBMessage::Data:
        case USBMessage::Status:
        default:
            break;
    }
    // Calculate packet count
    packetCount = size / usb->maxPacketSize;
    if (!packetCount)
        packetCount = 1;

    // Program channel registers.
    m_io.write(channelBase + Characteristics, (usb->endpointId << 11) |
                                              (dir << 15) |
                                              (usb->type << 18) |
                                              (usb->maxPacketSize) |
                                              (1 << 20) /* packetsPerFrame */ |
                                              (usb->deviceId << 22) );

    m_io.write(channelBase + SplitControl, usb->portAddress | (usb->hubAddress << 7) | (1 << 31));
    m_io.write(channelBase + ChannelDMA, data);
    m_io.write(channelBase + ChannelTransfer,
               size | (packetId << 29) | (packetCount << 19));

    // Clear and program interrupts
    m_io.write(channelBase + ChannelMask, 0);
    m_io.write(channelBase + ChannelInterrupt, 0xffffffff);
    m_io.write(channelBase + ChannelMask, 1 << 1); // ChannelHalted
    m_io.set(HostChannelMask, 1 << channelId);

    // Increase frame number. Set odd flag. Begin transfer.
    frameNumber = (m_io.read(HostFrameNumber) & 0xffff) + 1;
    m_io.set(channelBase + Characteristics, ((frameNumber & 1) << 29) | (1 << 31));
}

Error SynopsisUSB::transfer(const FileSystemMessage *msg,
                            USBMessage *usb)
{
    DEBUG("");

    switch (usb->state)
    {
        case USBMessage::Setup:
            startTransfer(0, msg, usb);
            break;

        case USBMessage::Data:
            break;

        case USBMessage::Status:
            break;

        default:
            break;
    }

    usb->state = USBMessage::Success;

    USBDescriptor::Device desc;
    desc.vendorId = 111;
    desc.productId = 222;

    VMCopy(msg->from, API::Write, (Address) &desc, usb->buffer, usb->size);
    return ESUCCESS;
}
