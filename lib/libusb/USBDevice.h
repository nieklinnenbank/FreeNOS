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

#ifndef __LIBUSB_USBDEVICE_H
#define __LIBUSB_USBDEVICE_H

#include <FreeNOS/System.h>
#include <Types.h>
#include <Device.h>
#include "USBTransfer.h"
#include "USBDescriptor.h"
#include "USBMessage.h"

/**
 * USB device class.
 */
class USBDevice : public Device
{
  public:

    /**
     * Constructor
     */
    USBDevice(const char *usbPath = "/usb");

    /**
     * Initialize the USBDevice.
     *
     * @return Result code
     */
    virtual Error initialize();

  protected:

    /**
     * Get device descriptor.
     */
    Error getDescriptor(USBDescriptor::Device *desc);

    /**
     * Send a control message.
     */
    Error controlMessage(const USBTransfer::DeviceRequest request,
                         const USBTransfer::Direction direction,
                         u16 value,
                         u16 index,
                         void *buffer,
                         Size size);

    /**
     * Perform a USB transfer.
     */
    Error transfer(USBMessage & msg);

  private:

    /** USB device identifier. */
    Address m_id;

    /** USB speed. */
    USBTransfer::Speed m_speed;

    /** USB transfer I/O file. */
    int m_transferFile;

    /** USB bus path. */
    String m_busPath;
};

#endif /* __LIBUSB_USBDEVICE_H */
