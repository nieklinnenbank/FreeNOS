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
#include <String.h>
#include <FileSystemClient.h>
#include <FileDescriptor.h>
#include "NetworkClient.h"
#include "ARP.h"
#include "ARPSocket.h"

NetworkClient::NetworkClient(const char *networkDevice)
{
    m_deviceName = networkDevice;
}

NetworkClient::~NetworkClient()
{
}

NetworkClient::Result NetworkClient::initialize()
{
    const FileSystemClient filesystem;
    Size numberOfMounts = 0;

    // Get a list of mounts
    const FileSystemMount *mounts = filesystem.getFileSystems(numberOfMounts);
    const FileSystemMount *match = 0;
    Size matchLen = 0;

    // Find closest matching device
    for (Size i = 0; i < numberOfMounts; i++)
    {
        const String mountPath(mounts[i].path);

        if (mountPath.length() > 0 && mountPath.compareTo("/network/", true, 9) == 0)
        {
            Size len = 0;

            for (Size j = 0; mounts[i].path[j+9] && m_deviceName[j]; j++, len++)
            {
                if (mounts[i].path[j+9] != m_deviceName[j])
                {
                    len = 0;
                    break;
                }
            }
            if (len >= matchLen)
            {
                match = &mounts[i];
                matchLen = len;
            }
        }
    }

    if (!match)
    {
        ERROR("network device not found: " << *m_deviceName);
        return IOError;
    }

    m_deviceName = match->path;
    return Success;
}

NetworkClient::Result NetworkClient::createSocket(const NetworkClient::SocketType type,
                                                  int *sock)
{
    const FileSystemClient fs;
    String path = m_deviceName;

    switch (type)
    {
        case ARP:
            path << "/arp/socket";
            break;

        case ICMP:
            path << "/icmp/factory";
            break;

        case UDP:
            path << "/udp/factory";
            break;

        default:
            return NotFound;
    }

    const FileSystem::Result result = fs.openFile(*path, *(Size *) sock);
    if (result != FileSystem::Success)
    {
        ERROR("failed to open socket factory at " << *path <<
              ": result = " << (int) result);
        return IOError;
    }

    return Success;
}

NetworkClient::Result NetworkClient::connectSocket(const int sock,
                                                   const IPV4::Address addr,
                                                   const u16 port)
{
    DEBUG("");
    return writeSocketInfo(sock, addr, port, Connect);
}

NetworkClient::Result NetworkClient::bindSocket(const int sock,
                                                const IPV4::Address addr,
                                                const u16 port)
{
    DEBUG("");
    return writeSocketInfo(sock, addr, port, Listen);
}

NetworkClient::Result NetworkClient::waitSocket(const NetworkClient::SocketType type,
                                                const int sock,
                                                const Size msecTimeout)
{
    const FileSystemClient fs;

    DEBUG("type = " << (int) type << " sock = " << sock);

    if (type != NetworkClient::UDP)
    {
        ERROR("protocol not supported: " << (int) type);
        return NetworkClient::NotSupported;
    }

    // Get file descriptor of the socket
    FileDescriptor::Entry *fd = FileDescriptor::instance()->getEntry(sock);
    if (!fd || !fd->open)
    {
        ERROR("failed to get FileDescriptor for socket " << sock << ": " << (fd ? "closed" : "not found"));
        return NetworkClient::NotFound;
    }

    // Prepare a wait set
    FileSystem::WaitSet waitSet;
    waitSet.inode     = fd->inode;
    waitSet.requested = FileSystem::Readable;
    waitSet.current   = 0;

    // Wait until the file is readable (has data)
    const FileSystem::Result waitResult = fs.waitFile(*m_deviceName, &waitSet, 1, msecTimeout);
    if (waitResult != FileSystem::Success)
    {
        if (waitResult == FileSystem::TimedOut)
        {
            DEBUG("operation timed out");
            return TimedOut;
        }
        else
        {
            ERROR("failed to wait for socket " << sock << " with inode " <<
                   waitSet.inode << ": result = " << (int) waitResult);
            return IOError;
        }
    }

    // File is ready for reading
    return Success;
}

NetworkClient::Result NetworkClient::writeSocketInfo(const int sock,
                                                     const IPV4::Address addr,
                                                     const u16 port,
                                                     const NetworkClient::SocketAction action)
{
    FileSystemClient fs;
    char buf[64];
    Size sz = sizeof(buf);

    DEBUG("");

    // Get file descriptor of the socket
    FileDescriptor::Entry *fd = FileDescriptor::instance()->getEntry(sock);
    if (!fd || !fd->open)
    {
        return NetworkClient::NotFound;
    }

    // Read socket factory. The factory will create
    // a new socket for us. We need to read the new file path
    const FileSystem::Result readResult = fs.readFile(sock, buf, &sz);
    if (readResult != FileSystem::Success)
    {
        ERROR("failed to read from socket " << sock <<
              ": result = " << (int) readResult);
        return IOError;
    }

    // Update the file descriptor inode and PID
    FileSystem::FileStat st;
    const FileSystem::Result statResult = fs.statFile(buf, &st);
    if (statResult != FileSystem::Success)
    {
        ERROR("failed to stat socket at path " << buf << ": result = " << (int) statResult);
        return IOError;
    }

    fd->inode = st.inode;
    fd->pid = st.pid;

    // Write address+port+action info to the socket
    SocketInfo info;
    info.address = addr;
    info.port    = port;
    info.action  = action;
    sz = sizeof(info);

    const FileSystem::Result writeResult = fs.writeFile(sock, &info, &sz);
    if (writeResult != FileSystem::Success)
    {
        ERROR("failed to write info to socket " << sock <<
              ": result = " << (int) writeResult);
        return IOError;
    }

    // Done
    return Success;
}

NetworkClient::Result NetworkClient::close(const int sock)
{
    const FileSystemClient fs;

    const FileSystem::Result result = fs.closeFile(sock);
    if (result != FileSystem::Success)
    {
        ERROR("failed to close socket " << sock << ": result = " << (int) result);
        return IOError;
    }

    return Success;
}
