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

#include <FreeNOS/API.h>
#include <Log.h>
#include <MemoryBlock.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "USBDevice.h"
#include "USBDescriptor.h"

USBDevice::USBDevice(u8 deviceId, const char *busPath)
    : Device(CharacterDeviceFile)
{
    m_id = deviceId;
    m_busPath = busPath;
    m_transferFile = -1;
    m_device = new USBDescriptor::Device;
    m_config = new USBDescriptor::Configuration;
    m_interface = new USBDescriptor::Interface;

    MemoryBlock::set(m_device, 0, sizeof(*m_device));
    MemoryBlock::set(m_config, 0, sizeof(*m_config));
    MemoryBlock::set(m_interface, 0, sizeof(*m_interface));
}

USBDevice::~USBDevice()
{
    delete m_device;
    delete m_config;
    delete m_interface;
}

Error USBDevice::initialize()
{
    DEBUG("");

    String usbTransfer;
    Address actualId = m_id;

    usbTransfer << *m_busPath << "/transfer";

    // Try to open the USB transfer file on the USB host controller
    if ((m_transferFile = ::open(*usbTransfer, O_RDWR)) < 0)
    {
        ERROR("failed to open " << *usbTransfer << ": " << strerror(errno) << "\n");
        return errno;
    }
    DEBUG("opened: " << *usbTransfer);

    // Every unconfigured USB device starts with address zero
    m_id = 0;

    // First retrieve the device descriptor in 8-bytes packet only
    // This is to figure out the maxPacketSize which is needed for transfers
    DEBUG("getting max packet size");
    if (getDeviceDescriptor(m_device, 8) != ESUCCESS)
    {
        ERROR("failed to get max packet size");
        return EIO;
    }

    DEBUG("maxPacketSize = " << m_device->maxPacketSize);

    // Retrieve device descriptor
    DEBUG("getting device descriptor");
    if (getDeviceDescriptor(m_device) != ESUCCESS)
    {
        ERROR("failed to get device descriptor");
        return EIO;
    }

    DEBUG("got descriptor: vendorId=" << m_device->vendorId <<
                        " productId=" << m_device->productId <<
                        " class=" << m_device->deviceClass <<
                        " subClass=" << m_device->deviceSubClass <<
                        " maxpacketsize=" << m_device->maxPacketSize <<
                        " protocol=" << m_device->deviceProtocol);

    // Retrieve configuration descriptor
    if (getConfigDescriptor(m_config) != ESUCCESS)
    {
        ERROR("failed to get configuration descriptor");
        return EIO;
    }

    // Set address of the USB device.
    if (setAddress(actualId) != ESUCCESS)
    {
        ERROR("failed to set address to: " << actualId);
        return EIO;
    }

    // Activate the first configuration on the USB device.
    if (setConfiguration(m_config->configurationValue) != ESUCCESS)
    {
        ERROR("failed to activate configurationValue: " << m_config->configurationValue);
        return EIO;
    }

    // Retrieve the full configuration descriptor, which
    // includes other descriptors (interface and endpoints)
    // just after the configuration descriptor.
#if 0
    getInterfaceDescriptor(m_interface);

    // Get all endpoint descriptors
    for (Size i = 1; i < m_interface->numEndpoints; i++)
    {
        USBDescriptor::Endpoint *ep = new USBDescriptor::Endpoint;
        MemoryBlock::set(ep, 0, sizeof(*ep));

        // Retrieve the endpoint descriptor
        getEndpointDescriptor(i, ep);
    }
#endif
    return ESUCCESS;
}

Error USBDevice::getDeviceDescriptor(USBDescriptor::Device *desc, Size size)
{
    DEBUG("");

    return controlMessage(USBTransfer::GetDescriptor,
                          USBTransfer::In,
                          USBTransfer::Standard,
                          USBTransfer::Device,
                          USBDescriptor::DeviceType << 8, 0,
                          desc, size);
}

Error USBDevice::getConfigDescriptor(USBDescriptor::Configuration *desc)
{
    DEBUG("");

    return controlMessage(USBTransfer::GetDescriptor,
                          USBTransfer::In,
                          USBTransfer::Standard,
                          USBTransfer::Device,
                          USBDescriptor::ConfigurationType << 8, 0,
                          desc, sizeof(*desc));
}

Error USBDevice::getInterfaceDescriptor(USBDescriptor::Interface *desc)
{
    DEBUG("");

    return controlMessage(USBTransfer::GetDescriptor,
                          USBTransfer::In,
                          USBTransfer::Standard,
                          USBTransfer::Device,
                          USBDescriptor::InterfaceType << 8, 0,
                          desc, sizeof(*desc));
}

Error USBDevice::getEndpointDescriptor(u8 endpointId, USBDescriptor::Endpoint *desc)
{
    DEBUG("endpointId = " << endpointId);

    return controlMessage(USBTransfer::GetDescriptor,
                          USBTransfer::In,
                          USBTransfer::Standard,
                          USBTransfer::Device,
                          USBDescriptor::EndpointType << 8, endpointId,
                          desc, sizeof(*desc));
}

Error USBDevice::setAddress(u8 address)
{
    DEBUG("address =" << address);

    // Send the request
    Error r = controlMessage(USBTransfer::SetAddress,
                             USBTransfer::Out,
                             USBTransfer::Standard,
                             USBTransfer::Device,
                             address, 0, 0, 0);

    // Set member, which is word aligned.
    if (r == ESUCCESS)
        m_id = address;

    return r;
}

Error USBDevice::setConfiguration(u8 configId)
{
    DEBUG("configId =" << configId);
    return controlMessage(USBTransfer::SetConfiguration,
                          USBTransfer::Out,
                          USBTransfer::Standard,
                          USBTransfer::Device,
                          configId, 0, 0, 0);
}

Error USBDevice::controlMessage(u8 request,
                                const USBTransfer::Direction direction,
                                const USBTransfer::RequestType type,
                                const USBTransfer::Recipient recipient,
                                u16 value,
                                u16 index,
                                void *buffer,
                                Size size)
{
    DEBUG("");

    USBMessage msg;
    msg.direction     = direction;
    msg.speed         = m_speed;
    msg.type          = USBTransfer::Control;
    msg.state         = USBMessage::Setup;
    msg.maxPacketSize = !m_device->maxPacketSize ? 8 : m_device->maxPacketSize;
    msg.hubAddress    = 1; // TODO
    msg.portAddress   = 1; // TODO

    msg.setup.requestType = (direction << 7) | (type << 5) | recipient;
    msg.setup.request     = request;
    msg.setup.value       = value;
    msg.setup.index       = index;
    msg.setup.length      = size;

    msg.deviceId      = m_id;
    msg.endpointId    = 0; // TODO: make parameter
    msg.size          = size;

    // Use the physical address of the buffer
    // TODO: for security this is not good. The USBDevice
    // can supply a malicious physical address, even inside the kernel.
    if (buffer)
    {
        Memory::Range range;
        range.virt = (Address) buffer;
        VMCtl(SELF, LookupVirtual, &range);
        msg.buffer = range.phys;
    }
    else
        msg.buffer = 0;

    // Perform control message transfer
    return submit(msg);
}

Error USBDevice::transfer(const USBTransfer::Type type,
                          const USBTransfer::Direction direction,
                          Address endpointId,
                          void *buffer,
                          Size size)
{
    DEBUG("");

    USBMessage msg;
    msg.direction     = direction;
    msg.speed         = m_speed;
    msg.type          = type;
    msg.state         = USBMessage::Status;
    msg.maxPacketSize = !m_device->maxPacketSize ? 8 : m_device->maxPacketSize;
    msg.deviceId      = m_id;
    msg.endpointId    = endpointId;
    msg.size          = size;

    // Use the physical address of the buffer
    // TODO: for security this is not good. The USBDevice
    // can supply a malicious physical address, even inside the kernel.
    Memory::Range range;
    range.virt = (Address) buffer;
    VMCtl(SELF, LookupVirtual, &range);
    msg.buffer = range.phys;

    // Perform control message transfer
    return submit(msg);
}

Error USBDevice::submit(USBMessage & msg)
{
    DEBUG("");

    // TODO: make this more efficient by doing a "special" readwrite() call? with IPCMessage(..., API::SendReceive, ...)

    // Make an USB transfer by writing the USB transfer file
    if (::write(m_transferFile, &msg, sizeof(msg)) != sizeof(msg))
    {
        ERROR("failed to write USB transfer file: " << strerror(errno));
        return EIO;
    }
    // Note that the USB controller will also implicitely write the
    // result of the transfer to the USBMessage buffer.
    DEBUG("transfer completed. USBMessage.state =" << (int)msg.state);

    return ESUCCESS;
}
