/*
 * Copyright (C) 2009 Niek Linnenbank
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

#include <FreeNOS/System.h>
#include <Types.h>
#include <Macros.h>
#include <Vector.h>
#include <ChannelClient.h>
#include <PageAllocator.h>
#include <PoolAllocator.h>
#include <FileSystemMount.h>
#include <FileSystemMessage.h>
#include <MemoryMap.h>
#include "FileDescriptor.h"
#include "stdlib.h"
#include "string.h"
#include "Runtime.h"
#include "unistd.h"
#include "libgen.h"
#include "fcntl.h"
#include "dirent.h"

/** List of constructors. */
extern void (*CTOR_LIST)();

/** List of destructors. */
extern void (*DTOR_LIST)();

/** FileSystem mounts table */
FileSystemMount mounts[FILESYSTEM_MAXMOUNTS];

/** Vector of FileDescriptors. */
// TODO: inefficient for memory usage. Replace this with an Index (array of data pointers).
Vector<FileDescriptor> files;

/** Current Directory String */
String currentDirectory;

void * __dso_handle = 0;

extern C void __aeabi_unwind_cpp_pr0()
{
}

extern C int __cxa_atexit(void (*func) (void *),
                          void * arg, void * dso_handle)
{
    return (0);
}

extern C int __aeabi_atexit()
{
    return 0;
}

extern C void __cxa_pure_virtual()
{
}

extern C void __stack_chk_fail(void)
{
}

extern C int raise(int sig)
{
    return 0;
}

void runConstructors()
{
    for (void (**ctor)() = &CTOR_LIST; ctor && *ctor; ctor++)
    {
        (*ctor)();
    }
}

void runDestructors()
{
    for (void (**dtor)() = &DTOR_LIST; dtor && *dtor; dtor++)
    {
        (*dtor)();
    }
}

void setupHeap()
{
    Allocator *parent;
    PoolAllocator *pool;
    Address heapAddr;
    Size parentSize;

    // TODO: inefficient. needs another kernel call. Make forkexec() have zero kernel calls and/or IPC until main() for
    // maximum efficiency.
    // TODO: ProcessCtl(SELF, InfoPID, &info);
    // map = info.map....
    Arch::MemoryMap map;

    PageAllocator alloc( map.range(MemoryMap::UserHeap).virt,
                         map.range(MemoryMap::UserHeap).size );

    // Pre-allocate 4 pages
    Size sz = PAGESIZE * 4;
    Address addr;
    alloc.allocate(&sz, &addr);

    // Allocate instance copy on vm pages itself
    heapAddr   = alloc.base();
    parent     = new (heapAddr) PageAllocator(&alloc);
    parentSize = sizeof(PageAllocator);

    // Make a pool
    pool = new (heapAddr + parentSize) PoolAllocator();
    pool->setParent(parent);
    
    // Set default allocator
    Allocator::setDefault(pool);
}

void setupRandomizer()
{
    ProcessID pid = getpid();
    Timer::Info timer;
    ProcessCtl(SELF, InfoTimer, (Address) &timer);

    ::srandom(pid + timer.ticks);
}

void setupChannels()
{
    ChannelClient *client = new ChannelClient();
    (void) client;

    ChannelClient::instance->setRegistry(new ChannelRegistry());
}

void setupMappings()
{
    // Fill the mounts table
    memset(mounts, 0, sizeof(FileSystemMount) * FILESYSTEM_MAXMOUNTS);
    strlcpy(mounts[0].path, "/mount", PATH_MAX);
    strlcpy(mounts[1].path, "/", PATH_MAX);
    mounts[0].procID  = MOUNTFS_PID;
    mounts[0].options = ZERO;
    mounts[1].procID  = ROOTFS_PID;
    mounts[1].options = ZERO;

    // Set currentDirectory
    currentDirectory = "/";

    // Load FileDescriptors.
    for (Size i = 0; i < FILE_DESCRIPTOR_MAX; i++)
    {
        FileDescriptor fd;
        fd.open = false;

        files.insert(fd);
    }
#warning Solve this, by passing the file descriptor, procinfo, etc as a parameter to entry(), constructed by the kernel
#warning If there was a parent, it would have passed the file descriptor table, argc/argv, memorymap, etc as an argument to ProcessCtl()

    // TODO: perhaps we can "bundle" the GetMounts() and ReadProcess() calls, so that
    // we do not need to send IPC message twice in this part (for mounts and getppid())

    // TODO: this is inefficient. It should take only one IPC request to retrieve these things from our parent. Or better, avoid it.

    // Get our parent ID
    ProcessID ppid = getppid();

    // Skip processes with no parent (e.g. from the BootImage)
    if (!ppid)
        return;

    // Inherit file descriptors, current directory, and more.
    FileSystemMessage msg;
    msg.from   = SELF;

    // NOTE: we "abuse" the FileSystemMessage for ipc with our parent...
    ChannelClient::instance->syncReceiveFrom(&msg, ppid);

    // Copy the file descriptors
    VMCopy(ppid, API::Read, (Address) files.vector(), (Address) msg.path, files.size() * sizeof(FileDescriptor));

    // Dummy reply, to tell our parent we received the fds.... very inefficient.
    ChannelClient::instance->syncSendTo(&msg, ppid);
}

ProcessID findMount(const char *path)
{
    FileSystemMount *m = ZERO;
    Size length = 0, len;
    char tmp[PATH_MAX];

    /* Is the path relative? */
    if (path[0] != '/')
    {
        getcwd(tmp, sizeof(tmp));
        snprintf(tmp, sizeof(tmp), "%s/%s", tmp, path);
    }
    else
        strlcpy(tmp, path, PATH_MAX);
        
    /* Find the longest match. */
    for (Size i = 0; i < FILESYSTEM_MAXMOUNTS; i++)
    {
        if (mounts[i].path[0])
        {
            len = strlen(mounts[i].path);
    
            /*
             * Only choose this mount, if it matches,
             * and is longer than the last match.
             */
            if (strncmp(tmp, mounts[i].path, len) == 0 && len > length)
            {
                length = len;
                m = &mounts[i];
            }
        }
    }
    // All done
    return m ? m->procID : ZERO;
}

void refreshMounts(const char *path)
{
    char tmp[PATH_MAX], number[16];
    int fd;
    struct dirent *dent;
    DIR *d;
    pid_t pid = getpid();

    // Skip for rootfs and mountfs
    if (pid == ROOTFS_PID || pid == MOUNTFS_PID)
        return;

    // Clear mounts table
    if (!path)
    {
        path = "/mount";
        MemoryBlock::set(&mounts[2], 0, sizeof(FileSystemMount) * (FILESYSTEM_MAXMOUNTS-2));
    }

    // Attempt to open the directory
    if (!(d = opendir(path)))
        return;

    // walk the /mounts recursively and refill the mounts table (starting from index 2)
    while ((dent = readdir(d)))
    {
        snprintf(tmp, sizeof(tmp), "/mount/%s", dent->d_name);

        switch (dent->d_type)
        {
            case DT_DIR:
                if (dent->d_name[0] != '.')
                    refreshMounts(tmp);
                break;

            case DT_REG:
                fd = open(tmp, O_RDONLY);
                if (fd >= 0)
                {
                    MemoryBlock::set(number, 0, sizeof(number));

                    if (read(fd, number, sizeof(number)) > 0)
                    {
                        pid_t pid = atoi(number);

                        // Append to the mounts table
                        for (Size i = 0; i < FILESYSTEM_MAXMOUNTS; i++)
                        {
                            if (!mounts[i].path[0])
                            {
                                mounts[i].procID = pid;
                                strlcpy(mounts[i].path, tmp+6, PATH_MAX);
                                break;
                            }
                        }
                    }
                    close(fd);
                }
                break;

            default:
                break;
        }
    }
    closedir(d);
}

ProcessID findMount(int fildes)
{
    return files.get(fildes) ? files.get(fildes)->mount : ZERO;
}

FileSystemMount * getMounts()
{
    return mounts;
}

Vector<FileDescriptor> * getFiles()
{
    return &files;
}

String * getCurrentDirectory()
{
    return &currentDirectory;
}

extern C void SECTION(".entry") _entry() 
{
    int ret, argc;
    char *arguments;
    char **argv;
    SystemInformation info;

    /* Clear BSS */
    extern Address __bss_start, __bss_end;
    Address bss_size = &__bss_end - &__bss_start;
    MemoryBlock::set(&__bss_start, 0, bss_size);

    /* Setup the heap, C++ constructors and default mounts. */
    setupHeap();
    runConstructors();
    setupChannels();
    setupMappings();
    setupRandomizer();

    /* Allocate buffer for arguments. */
    argc = 0;
    argv = (char **) new char[ARGV_COUNT];
    arguments = (char *) ARGV_ADDR;

    /* Fill in arguments list. */
    while (argc < ARGV_COUNT && *arguments)
    {
        argv[argc] = arguments;
        arguments += ARGV_SIZE;
        argc++;
    }
    /* Pass control to the program. */
    ret = main(argc, argv);

    /* Terminate execution. */
    runDestructors();
    exit(ret);
}
