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

/*
 * Blocking-reads for /path/to/usb/controller/$eventfile
 * The USBController has an EventFile for that, which returns 
 * when the interrupt occurred, perhaps even with the data describing what the event was.
 *
 * The USB Hub powers on, enables, resets, and configures devices+ports when
 * a port status changes. It spawns new device drivers which then do direct I/O with the
 * controller.
 *
 * Interaction with USBController for controlling the ports is using USB Control messages, e.g.:
 *   /usb/control
 * of:
 *   /usb/transfer
 */

#include <Log.h>
#include "USBHub.h"

USBHub::USBHub(const char *usbPath)
    : USBDevice(usbPath)
{
    DEBUG("");
}

Error USBHub::initialize()
{
    Error r;

    if ((r = USBDevice::initialize()) != ESUCCESS)
        return r;

    DEBUG("");

    USBDescriptor::Device desc;

    DEBUG("getting descriptor");
    getDescriptor(&desc);

    DEBUG("got descriptor: vendorId=" << desc.vendorId << " productId=" << desc.productId);
    return ESUCCESS;
}
