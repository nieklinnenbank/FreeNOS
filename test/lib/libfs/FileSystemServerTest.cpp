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

#include <FreeNOS/Constant.h>
#include <FreeNOS/API/ProcessID.h>
#include <TestCase.h>
#include <TestRunner.h>
#include <TestInt.h>
#include <TestMain.h>
#include <MemoryBlock.h>
#include <MemoryChannel.h>
#include <PseudoFile.h>
#include <FileSystemServer.h>

/**
 * Filesystem for testing purposes in this file
 */
class DummyFileSystem : public FileSystemServer
{
  public:

    DummyFileSystem(Directory *root, const char *mountPath)
        : FileSystemServer(root, mountPath)
        , m_kernelProducer(Channel::Producer, sizeof(ProcessEvent))
        , m_clientProducer(new MemoryChannel(Channel::Producer, sizeof(FileSystemMessage)))
        , m_clientConsumer(new MemoryChannel(Channel::Consumer, sizeof(FileSystemMessage)))
        , m_rootRequestProducer(new MemoryChannel(Channel::Producer, sizeof(FileSystemMessage)))
        , m_rootResponseConsumer(new MemoryChannel(Channel::Consumer, sizeof(FileSystemMessage)))
        , m_rootResponseProducer(Channel::Producer, sizeof(FileSystemMessage))
    {
        assert(m_pid != ROOTFS_PID);

        // Clear message pages
        MemoryBlock::set(m_pages, 0, sizeof(m_pages));

        // Add kernel channel
        m_kernelEvent.setVirtual((Address) m_pages, ((Address) m_pages) + PAGESIZE);
        m_kernelProducer.setVirtual((Address) m_pages, ((Address) m_pages) + PAGESIZE);

        // Add client channel
        m_clientConsumer->setVirtual(((Address) m_pages) + (PAGESIZE * 2u),
                                     ((Address) m_pages) + (PAGESIZE * 3u));
        m_clientProducer->setVirtual(((Address) m_pages) + (PAGESIZE * 2u),
                                     ((Address) m_pages) + (PAGESIZE * 3u));
        m_registry.registerProducer(m_pid, m_clientProducer);
        m_registry.registerConsumer(m_pid, m_clientConsumer);

        // Add rootfs channel
        m_rootRequestProducer->setVirtual(((Address) m_pages) + (PAGESIZE * 4u),
                                          ((Address) m_pages) + (PAGESIZE * 5u));
        m_rootResponseConsumer->setVirtual(((Address) m_pages) + (PAGESIZE * 6u),
                                           ((Address) m_pages) + (PAGESIZE * 7u));
        m_rootResponseProducer.setVirtual(((Address) m_pages) + (PAGESIZE * 6u),
                                          ((Address) m_pages) + (PAGESIZE * 7u));
        m_registry.registerProducer(ROOTFS_PID, m_rootRequestProducer);
        m_registry.registerConsumer(ROOTFS_PID, m_rootResponseConsumer);
    }

    virtual ~DummyFileSystem()
    {
        m_registry.unregisterProducer(m_pid);
        m_registry.unregisterConsumer(m_pid);
        m_registry.unregisterProducer(ROOTFS_PID);
        m_registry.unregisterConsumer(ROOTFS_PID);
    }

    static u8 m_pages[PAGESIZE * 2 * 4];
    MemoryChannel m_kernelProducer;
    MemoryChannel *m_clientProducer;
    MemoryChannel *m_clientConsumer;
    MemoryChannel *m_rootRequestProducer;
    MemoryChannel *m_rootResponseConsumer;
    MemoryChannel m_rootResponseProducer;
};

u8 DummyFileSystem::m_pages[PAGESIZE * 2 * 4];

TestCase(FileSystemServerConstruct)
{
    Directory *root = new Directory(1);
    DummyFileSystem fs(root, "/mnt");

    // Validate members
    testAssert(fs.m_root != ZERO);
    testAssert(fs.m_root->file == root);
    testString(*fs.m_root->name, "/");
    testAssert(fs.m_mounts == ZERO);
    testAssert(fs.m_requests != ZERO);
    testString(fs.getMountPath(), "/mnt");

    return OK;
}

TestCase(FileSystemServerMount)
{
    DummyFileSystem fs(new Directory(1), "/mnt");

    // Prepare response
    FileSystemMessage msg;
    msg.type = ChannelMessage::Response;
    msg.result = FileSystem::Success;
    testAssert(fs.m_rootResponseProducer.write(&msg) == Channel::Success);

    // Send request
    testAssert(fs.mount() == FileSystem::Success);
    return OK;
}

TestCase(FileSystemServerRegisterFile)
{
    const char *path = "/myfile.txt";
    FileSystemServer fs(new Directory(1), "/mnt");

    // Initial state is no files present
    testAssert(fs.findFileCache(path) == ZERO);

    // Add the file
    File *file = new File(fs.getNextInode());
    const FileSystem::Result result = fs.registerFile(file, path);

    // Verify the file is added
    testAssert(result == FileSystem::Success);
    testAssert(fs.findFileCache(path) != ZERO);
    testAssert(fs.findFileCache(path)->file == file);
    testString(*fs.findFileCache(path)->name, "myfile.txt");

    return OK;
}

TestCase(FileSystemServerStatFile)
{
    DummyFileSystem fs(new Directory(1), "/mnt");

    // Prepare message for non-existing file
    String path("/mnt/myfile.txt");
    FileSystem::FileStat st;
    FileSystemMessage msg;
    msg.from   = fs.m_pid;
    msg.action = FileSystem::StatFile;
    msg.buffer = *path;
    msg.stat   = &st;
    msg.size   = sizeof(st);
    msg.offset = 0;

    // Process the message
    fs.pathHandler(&msg);

    // Receive response
    testAssert(fs.m_clientConsumer->read(&msg) == Channel::Success);
    testAssert(msg.result == FileSystem::NotFound);

    // Add the file
    File *file = new File(fs.getNextInode(), FileSystem::CharacterDeviceFile, 1, 2);
    testAssert(fs.registerFile(file, "myfile.txt") == FileSystem::Success);

    // Send another request
    fs.pathHandler(&msg);

    // Receive response
    testAssert(fs.m_clientConsumer->read(&msg) == Channel::Success);
    testAssert(msg.result == FileSystem::Success);
    testAssert(st.type == FileSystem::CharacterDeviceFile);
    testAssert(st.userID == 1);
    testAssert(st.groupID == 2);
    testAssert(st.access == FileSystem::OwnerRWX);
    testAssert(st.size == 0);

    return OK;
}

TestCase(FileSystemServerReadFile)
{
    DummyFileSystem fs(new Directory(1), "/mnt");

    // Prepare message for non-existing file
    char buf[128];
    FileSystemMessage msg;
    msg.from   = fs.m_pid;
    msg.action = FileSystem::ReadFile;
    msg.inode  = 12356;
    msg.buffer = buf;
    msg.size   = sizeof(buf);
    msg.offset = 0;

    // Process the message
    fs.pathHandler(&msg);

    // Receive response
    testAssert(fs.m_clientConsumer->read(&msg) == Channel::Success);
    testAssert(msg.result == FileSystem::NotFound);

    // Add the file
    const u32 inode = fs.getNextInode();
    File *file = new PseudoFile(inode, "mydata");
    testAssert(fs.registerFile(file, "myfile.txt") == FileSystem::Success);

    // Send another request
    msg.inode = inode;
    fs.pathHandler(&msg);

    // Receive response
    testAssert(fs.m_clientConsumer->read(&msg) == Channel::Success);
    testAssert(msg.result == FileSystem::Success);
    testAssert(msg.size == 6);

    // Verify content
    buf[msg.size] = 0;
    testString(buf, "mydata");

    return OK;
}

TestCase(FileSystemServerWriteFile)
{
    DummyFileSystem fs(new Directory(1), "/mnt");

    // Prepare message for non-existing file
    String buf("something");
    char buf2[128];
    FileSystemMessage msg;
    msg.from   = fs.m_pid;
    msg.action = FileSystem::WriteFile;
    msg.inode  = 54321;
    msg.buffer = *buf;
    msg.size   = buf.length();
    msg.offset = 0;

    // Process the message
    fs.pathHandler(&msg);

    // Receive response
    testAssert(fs.m_clientConsumer->read(&msg) == Channel::Success);
    testAssert(msg.result == FileSystem::NotFound);

    // Add the file
    const u32 inode = fs.getNextInode();
    File *file = new PseudoFile(inode);
    testAssert(fs.registerFile(file, "myfile.txt") == FileSystem::Success);

    // Send another request
    msg.inode = inode;
    fs.pathHandler(&msg);

    // Receive response
    testAssert(fs.m_clientConsumer->read(&msg) == Channel::Success);
    testAssert(msg.result == FileSystem::Success);

    // Read out the file again
    msg.action = FileSystem::ReadFile;
    msg.buffer = buf2;
    msg.size   = sizeof(buf2);
    fs.pathHandler(&msg);

    // Receive response
    testAssert(fs.m_clientConsumer->read(&msg) == Channel::Success);
    testAssert(msg.result == FileSystem::Success);
    testAssert(msg.size == 9);

    // Verify content
    msg.buffer[msg.size] = 0;
    testString(buf2, "something");

    return OK;
}

TestCase(FileSystemServerDeleteFile)
{
    DummyFileSystem fs(new Directory(1), "/mnt");

    // Prepare message for non-existing file
    String path("/mnt/myfile.txt");
    char buf2[128];
    FileSystemMessage msg;
    msg.from   = fs.m_pid;
    msg.action = FileSystem::DeleteFile;
    msg.buffer = *path;

    // Process the message
    fs.pathHandler(&msg);

    // Receive response
    testAssert(fs.m_clientConsumer->read(&msg) == Channel::Success);
    testAssert(msg.result == FileSystem::NotFound);

    // Add the file
    const u32 inode = fs.getNextInode();
    File *file = new PseudoFile(inode);
    testAssert(fs.registerFile(file, "myfile.txt") == FileSystem::Success);

    // Send another request
    fs.pathHandler(&msg);

    // Receive response
    testAssert(fs.m_clientConsumer->read(&msg) == Channel::Success);
    testAssert(msg.result == FileSystem::Success);

    // Read out the file again (should be removed)
    msg.action = FileSystem::ReadFile;
    msg.inode = inode;
    msg.buffer = buf2;
    msg.size   = sizeof(buf2);
    fs.pathHandler(&msg);

    // Receive response
    testAssert(fs.m_clientConsumer->read(&msg) == Channel::Success);
    testAssert(msg.result == FileSystem::NotFound);

    return OK;
}
