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

#include "DeviceServer.h"

DeviceServer::DeviceServer(const char *path)
    : FileSystem(path)
{
    m_interrupts.fill(ZERO);
}

DeviceServer::~DeviceServer()
{
}

Error DeviceServer::initialize()
{
    setRoot(new Directory());
    mount();
    return ESUCCESS;
}

// TODO: avoid using format strings. Replace with String.

void DeviceServer::registerDevice(Device *dev, const char *path, ...)
{
    char buf[PATHLEN];
    va_list args;

    // Add to the list of Devices
    m_devices.insert(dev);

    // Initialize the device
    dev->initialize();

    // Add to the filesystem cache
    va_start(args, path);
    vsnprintf(buf, sizeof(buf), path, args);
    va_end(args);

    va_start(args, path);
    insertFileCache(dev, path, args);
    va_end(args);

    // Also add to the parent directory
    FileSystemPath p((char *)buf);
    Directory *parent;

    if (p.parent())
        parent = (Directory *) findFileCache(**p.parent())->file;
    else
        parent = (Directory *) m_root->file;

    parent->insert(dev->getType(), **p.base());
}

void DeviceServer::registerInterrupt(Device *dev, Size vector)
{
    if (!m_interrupts[vector])
    {
        m_interrupts.insert(vector, new List<Device *>);
    }
    m_interrupts[vector]->append(dev);

    // Register to kernel
    ProcessCtl(SELF, WatchIRQ, vector);
    ProcessCtl(SELF, EnableIRQ, vector);

    // Register interrupt handler
    addIRQHandler(vector, (IRQHandlerFunction) &DeviceServer::interruptHandler);
}

void DeviceServer::interruptHandler(InterruptMessage *msg)
{
    List<Device *> *lst = m_interrupts.at(msg->vector);

    // Do we have any Devices with this interrupt vector?
    if (lst)
    {
        // Loop all Devices of interest. Invoke callback.
        for (ListIterator<Device *> i(lst); i.hasCurrent(); i++)
        {
            i.current()->interrupt(msg->vector);
        }
    }

    // Retry any pending requests, if any
    List<FileSystemMessage *> *req = m_requests;
    m_requests = new List<FileSystemMessage *>();

    if (req)
    {
        for (ListIterator<FileSystemMessage *> i(req); i.hasCurrent(); i++)
        {
            pathHandler(i.current());
        }
        delete req;
    }
}
