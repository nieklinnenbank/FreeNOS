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
#include <unistd.h>
#include <string.h>
#include "USBHub.h"

USBHub::USBHub(u8 deviceId, const char *usbPath)
    : USBDevice(deviceId, usbPath)
{
    DEBUG("");

    m_hub = new USBDescriptor::Hub;
}

USBHub::~USBHub()
{
    delete m_hub;
}

Error USBHub::initialize()
{
    Error r;

    if ((r = USBDevice::initialize()) != ESUCCESS)
    {
        ERROR("failed to initialize USB device for Hub");
        return r;
    }

    DEBUG("get hub descriptor");
    r = controlMessage(GetDescriptor,
                       USBTransfer::In,
                       USBTransfer::Class,
                       USBTransfer::Device,
                       USBDescriptor::HubType << 8, 0,
                       m_hub, sizeof(*m_hub));
    if (r != ESUCCESS)
    {
        ERROR("failed to get Hub descriptor");
        return r;
    }

    DEBUG("found " << m_hub->numPorts << " ports");
    DEBUG("characteristics: " << m_hub->hubCharacteristics);

    // TODO: each port gets USB device id of myself + (portnumber)
    // if the device is a HUB, ask our PARENT for a new deviceId range.
    // in case we are the root HUB, we can figure it out, since we started with the full 255 range.
    // a HUB needs to manage its childs anyway, since in case the HUB is disconnected it must KILL each child too.
    // thus if a child is terminated, the HUB can reclaim the deviceId range too.

    DEBUG("power-on ports");
    for (Size i = 1; i < m_hub->numPorts + 1u; i++)
    {
        if (setPortFeature(i, PortPower) != ESUCCESS)
        {
            ERROR("failed to power-on port: " << i);
        }
    }
    // TODO: assume a device is attached on the first port for testing
    if (portAttach(1) != ESUCCESS)
    {
        ERROR("failed to attach port 1");
    }

    /*
     * Use an interrupt transfer to find out if a port-change happend:
     *
     * "Everything else the hub driver does happens asynchronously as a response to a status
change request being completed. Every USB hub has exactly one interrupt IN endpoint
called the status change endpoint. The hub responds on this endpoint whenever the
status of the hub or one of the hub.s ports has changed. for example, when a USB
device has been connected or disconnected from a port"
     *
     * on device attached:
     * - reset port
     * - enable port
     * - then start a new device driver with the assigned usb device ID.
     *   which will do the full USB init (get descriptors, set address, set config)
     */

    return ESUCCESS;
}

Error USBHub::setPortFeature(u8 port, USBHub::PortFeature feature)
{
    DEBUG("port =" << port << " feature=" << (int)feature);

    return controlMessage(SetFeature,
                          USBTransfer::Out,
                          USBTransfer::Class,
                          USBTransfer::Other,
                          feature, port, 0, 0);
}

Error USBHub::portAttach(u8 port)
{
    DEBUG("");

    DEBUG("enable port");
    setPortFeature(port, PortReset);

    sleep(30);

    // Get descriptor of the next device
    u8 myId = m_id;
    USBDescriptor::Device *desc = new USBDescriptor::Device;
    m_id = 0;
    if (getDeviceDescriptor(desc) != ESUCCESS)
    {
        ERROR("failed to get device descriptor at port 1");
    }
    else
    {
        DEBUG("device on port " << port << " is: vendorId=" << desc->vendorId <<
                            " productId=" << desc->productId <<
                             " class=" << desc->deviceClass <<
                             " subclass=" << desc->deviceSubClass <<
                             " manufacturer=" << desc->manufacturer << 
                             " product=" << desc->product <<
                             " descType=" << desc->descriptorType);
    }
    m_id = myId;

    // TODO: hardcoded assumption: SMSC95xx is on port 1 (or 3?) (raspberry pi)
    if (desc->vendorId == 1060 && desc->productId == 60416)
    {
        const char * argv[] = { "/server/network/smsc95xx/server", 0 };

        if (forkexec(argv[0], argv) == -1)
        {
            ERROR("failed to start USB device driver: " << strerror(errno));
            return EIO;
        }
        DEBUG("USB device driver started for port =" << port);
    }
    else
    {
        ERROR("no SMSC95xx found on port 1");
    }
    return ESUCCESS;
}
