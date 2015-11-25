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

#include <Log.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "USBDevice.h"

USBDevice::USBDevice(const char *busPath)
    : Device(CharacterDeviceFile)
{
    m_id = 0; // TODO???
    m_busPath = busPath;
    m_transferFile = -1;
}

Error USBDevice::initialize()
{
    DEBUG("");

    String usbTransfer;

    usbTransfer << *m_busPath << "/transfer";

    // Try to open the USB transfer file on the USB host controller
    if ((m_transferFile = ::open(*usbTransfer, O_RDWR)) < 0)
    {
        ERROR("failed to open " << *usbTransfer << ": " << strerror(errno) << "\n");
        return errno;
    }
    DEBUG("opened: " << *usbTransfer);
    return ESUCCESS;
}

Error USBDevice::getDescriptor(USBDescriptor::Device *desc)
{
    DEBUG("");

    controlMessage(USBTransfer::GetDescriptor,
                   USBTransfer::In, 0, 0,
                   desc, sizeof(*desc));

    return ESUCCESS;
}

Error USBDevice::controlMessage(const USBTransfer::DeviceRequest request,
                                const USBTransfer::Direction direction,
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
    msg.maxPacketSize = 8;
    msg.hubAddress    = 0; // TODO
    msg.portAddress   = 0; // TODO

    msg.setup.requestType = (direction << 7) | (USBTransfer::Standard << 5) | (USBTransfer::Device);
    msg.setup.request     = request;
    msg.setup.value       = value;
    msg.setup.index       = index;
    msg.setup.length      = size;

    msg.deviceId      = m_id;
    msg.endpointId    = 0; // TODO: ????
    msg.buffer        = (Address) buffer;
    msg.size          = size;
    transfer(msg);

    return ESUCCESS;
}

Error USBDevice::transfer(USBMessage & msg)
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
