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

#include <FreeNOS/Config.h>
#include <TestRunner.h>
#include <TestInt.h>
#include <TestCase.h>
#include <TestMain.h>
#include <ByteOrder.h>
#include <Channel.h>
#include <ApplicationLauncher.h>
#include <NetworkServer.h>
#include <Loopback.h>

class DummyChannel : public Channel
{
  public:
    DummyChannel(Channel::Mode mode, const Size sz)
        : Channel(mode, sz)
    {
    }

    virtual Result read(void *buffer)
    {
        MemoryBlock::set(buffer, 0, sizeof(FileSystemMessage));
        FileSystemMessage *msg = (FileSystemMessage *) buffer;
        msg->result = FileSystem::Success;
        return Channel::Success;
    }

    virtual Result write(const void *buffer)
    {
        return Channel::Success;
    }
};

TestCase(LoopbackInitialize)
{
    const char *mountPath = "/networktest/loopback";
    NetworkServer server(mountPath);
    Loopback *loop = new Loopback(server.getNextInode(), server);

    // Initialize
    server.registerNetworkDevice(loop);
    server.m_mountPath = ZERO;
    testAssert(server.initialize() == FileSystem::Success);
    server.m_mountPath = mountPath;

    return OK;
}

TestCase(LoopbackUdpPing)
{
    const char *mountPath = "/networktest/loopback";
    NetworkServer server(mountPath);
    Loopback *loop = new Loopback(server.getNextInode(), server);

    // Register dummy channels
    server.m_registry.registerProducer(server.m_pid, new DummyChannel(Channel::Producer, sizeof(FileSystemMessage)));
    server.m_registry.registerConsumer(server.m_pid, new DummyChannel(Channel::Consumer, sizeof(FileSystemMessage)));

#ifdef __HOST__
    server.m_registry.registerProducer(ROOTFS_PID, new DummyChannel(Channel::Producer, sizeof(FileSystemMessage)));
    server.m_registry.registerConsumer(ROOTFS_PID, new DummyChannel(Channel::Consumer, sizeof(FileSystemMessage)));
#endif /* __HOST__ */

    // Initialize the server
    server.registerNetworkDevice(loop);
    server.m_mountPath = ZERO;
    testAssert(server.initialize() == FileSystem::Success);
    server.m_mountPath = mountPath;

    // Stat the UDP socket factory
    FileSystem::FileStat st;
    FileSystemMessage msg;
    MemoryBlock::set(&msg, 0, sizeof(msg));
    MemoryBlock::set(&st, 0, sizeof(st));
    msg.from = server.m_pid;
    msg.action = FileSystem::StatFile;
    msg.buffer = (char *) "/networktest/loopback/udp/factory";
    msg.stat = &st;
    msg.size = sizeof(st);
    msg.offset = 0;

    // Process the message
    server.pathHandler(&msg);
    const u32 factoryInode = st.inode;
    testAssert(msg.result == FileSystem::Success);
    testAssert(st.type == FileSystem::RegularFile);
    testAssert(factoryInode != 0);

    // Read new UDP socket path message
    char buf[128];
    MemoryBlock::set(&buf, 0, sizeof(buf));
    msg.action = FileSystem::ReadFile;
    msg.buffer = buf;
    msg.inode = factoryInode;
    msg.size = sizeof(buf);

    // Process the message
    server.pathHandler(&msg);
    String str(buf, false);

    testAssert(msg.result == FileSystem::Success);
    testAssert(str.equals("/networktest/loopback/udp/0"));
    testAssert(loop->m_udp->m_sockets.count() == 1);

    // Stat UDP socket to retrieve inode
    MemoryBlock::set(&st, 0, sizeof(st));
    msg.action = FileSystem::StatFile;
    msg.buffer = (char *) "/networktest/loopback/udp/0";
    msg.stat = &st;
    msg.size = sizeof(st);

    // Process the message
    server.pathHandler(&msg);
    const u32 udp0 = st.inode;
    testAssert(msg.result == FileSystem::Success);
    testAssert(st.type == FileSystem::RegularFile);
    testAssert(udp0 != 0);

    // Bind the UDP socket to a port
    msg.action = FileSystem::WriteFile;
    msg.buffer = buf;
    msg.size = sizeof(NetworkClient::SocketInfo);
    msg.inode = udp0;

    NetworkClient::SocketInfo *info = (NetworkClient::SocketInfo *) &buf;
    info->address = IPV4::toAddress("127.0.0.1");
    info->port = 12345;
    info->action = NetworkClient::Listen;

    // Process the message
    server.pathHandler(&msg);
    testAssert(msg.result == FileSystem::Success);

    // Create a second UDP socket
    msg.from = server.m_pid;
    msg.action = FileSystem::ReadFile;
    msg.inode = factoryInode;
    msg.buffer = buf;
    msg.size = sizeof(buf);
    msg.offset = 0;

    // Process the message
    server.pathHandler(&msg);
    testAssert(msg.result == FileSystem::Success);
    testAssert(str.equals("/networktest/loopback/udp/1"));
    testAssert(loop->m_udp->m_sockets.count() == 2);

    // Stat second UDP socket to retrieve inode
    MemoryBlock::set(&st, 0, sizeof(st));
    msg.action = FileSystem::StatFile;
    msg.buffer = (char *) "/networktest/loopback/udp/1";
    msg.stat = &st;
    msg.size = sizeof(st);

    // Process the message
    server.pathHandler(&msg);
    const u32 udp1 = st.inode;
    testAssert(msg.result == FileSystem::Success);
    testAssert(st.type == FileSystem::RegularFile);
    testAssert(udp1 != 0);
    testAssert(udp1 != udp0);

    // Bind the second UDP socket to a port
    msg.action = FileSystem::WriteFile;
    msg.buffer = buf;
    msg.inode = udp1;
    msg.size = sizeof(NetworkClient::SocketInfo);
    info->address = IPV4::toAddress("127.0.0.1");
    info->port = 54321;
    info->action = NetworkClient::Listen;

    // Process the message
    server.pathHandler(&msg);
    testAssert(msg.result == FileSystem::Success);

    // Send a UDP packet from the second socket to the first
    const char *payload = "testing 1234";
    msg.from = server.m_pid;
    msg.action = FileSystem::WriteFile;
    msg.inode = udp1;
    msg.buffer = buf;
    msg.size = sizeof(buf);
    msg.offset = 0;

    // Fill the packet contents
    info->address = IPV4::toAddress("127.0.0.1");
    info->port = 12345;
    info->action = NetworkClient::SendSingle;
    MemoryBlock::copy(info + 1, payload, String::length(payload));

    // Process the message two times: first for ARP lookup, second for actual packet
    server.pathHandler(&msg);
    testAssert(msg.result == FileSystem::Success);
    server.retryRequests();
    testAssert(msg.result == FileSystem::Success);

    // Receive the packet
    MemoryBlock::set(buf, 0, sizeof(buf));
    msg.from = server.m_pid;
    msg.action = FileSystem::ReadFile;
    msg.inode = udp0;
    msg.buffer = buf;
    msg.size = sizeof(buf);
    msg.offset = 0;

    // Process the message
    server.pathHandler(&msg);
    testAssert(msg.result == FileSystem::Success);
    testAssert(info->address == IPV4::toAddress("127.0.0.1"));
    testAssert(info->port == 54321);
    testAssert(MemoryBlock::compare(info + 1, payload, String::length(payload)));

    return OK;
}

#ifndef __HOST__
TestCase(LoopbackArpPing)
{
    // Launch ping program
    const char *args[] = { "/bin/netping", "-a", "lo", "127.0.0.1", ZERO };
    ApplicationLauncher ping(TESTROOT "/bin/netping", args);

    // Start the program
    const ApplicationLauncher::Result resultCode = ping.exec();
    testAssert(resultCode == ApplicationLauncher::Success);

    // Wait for program to finish
    const ApplicationLauncher::Result waitResult = ping.wait();
    testAssert(waitResult == ApplicationLauncher::Success || waitResult == ApplicationLauncher::NotFound);
    testAssert(ping.getExitCode() == 0);

    // Done
    return OK;
}

TestCase(LoopbackIcmpPing)
{
    // Launch ping program
    const char *args[] = { "/bin/netping", "-i", "lo", "127.0.0.1", ZERO };
    ApplicationLauncher ping(TESTROOT "/bin/netping", args);

    // Start the program
    const ApplicationLauncher::Result resultCode = ping.exec();
    testAssert(resultCode == ApplicationLauncher::Success);

    // Wait for program to finish
    const ApplicationLauncher::Result waitResult = ping.wait();
    testAssert(waitResult == ApplicationLauncher::Success || waitResult == ApplicationLauncher::NotFound);
    testAssert(ping.getExitCode() == 0);

    // Done
    return OK;
}
#endif /* __HOST__ */
