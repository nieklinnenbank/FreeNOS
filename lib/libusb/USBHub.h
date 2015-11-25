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

#ifndef __LIBUSB_USBHUB_H
#define __LIBUSB_USBHUB_H

#include <Types.h>
#include "USBDevice.h"

/**
 * USB Hub driver.
 */
class USBHub : public USBDevice
{
  public:

    /**
     * USB Hub current status and changed status format.
     */
    typedef struct Status
    {
        u16 current;
        u16 changed;
    }
    Status;

    /**
     * USB Hub standard requests.
     */
    enum RequestType
    {
        GetStatus        = 0,
        ClearFeature     = 1,
        SetFeature       = 3,
        GetDescriptor    = 6,
        SetDescriptor    = 7,
        ClearTTBuffer    = 8,
        ResetTT          = 9,
        GetTTState       = 10,
        StopTT           = 11
    };

    /**
     * Constructor
     */
    USBHub(const char *usbPath = "/usb");

    /**
     * Initialize the hub.
     *
     * @return Result code
     */
    virtual Error initialize();

  private:

};

#endif /* __LIBUSB_USBHUB_H */
