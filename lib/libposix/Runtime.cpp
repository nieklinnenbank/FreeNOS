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

#include <FreeNOS/API.h>
#include <FreeNOS/System.h>
#include <Types.h>
#include <Macros.h>
#include <Vector.h>
#include <PageAllocator.h>
#include <PoolAllocator.h>
#include <MemoryAllocator.h>
#include <FileSystemMount.h>
#include <CoreServer.h>
#include "FileDescriptor.h"
#include "stdlib.h"
#include "string.h"
#include "Runtime.h"
#include "unistd.h"

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
    Arch::Memory mem; // TODO: inefficient. needs another kernel call for SystemInfo()...

    /* Only the core server allocates directly. */
    if (ProcessCtl(SELF, GetPID) == CORESRV_PID)
    {
        MemoryAllocator alloc( mem.range(Memory::UserHeap).virt,
                               mem.range(Memory::UserHeap).size );

        // Pre-allocate 4 pages
        Size sz = PAGESIZE * 4;
        Address addr;
        alloc.allocate(&sz, &addr);

        // Allocate instance copy on vm pages itself
        heapAddr   = alloc.base();
        parent     = new (heapAddr) MemoryAllocator(&alloc);
        parentSize = sizeof(MemoryAllocator);
    }
    else
    {
        PageAllocator alloc( mem.range(Memory::UserHeap).virt,
                             mem.range(Memory::UserHeap).size );

        // Pre-allocate 4 pages
        Size sz = PAGESIZE * 4;
        Address addr;
        alloc.allocate(&sz, &addr);

        // Allocate instance copy on vm pages itself
        heapAddr   = alloc.base();
        parent     = new (heapAddr) PageAllocator(&alloc);
        parentSize = sizeof(PageAllocator);
    }
    // Make a pool
    pool = new (heapAddr + parentSize) PoolAllocator();
    pool->setParent(parent);
    
    // Set default allocator
    Allocator::setDefault(pool);
}

void setupMappings()
{
    // The CoreServer does not need to setup mappings
    if (getpid() == CORESRV_PID)
        return;

    // Ask CoreServer for FileSystemMounts table
    CoreMessage msg;
    msg.action = GetMounts;
    msg.mounts = mounts;
    msg.type   = IPCType;
    msg.from   = SELF;
    IPCMessage(CORESRV_PID, API::SendReceive, &msg, sizeof(msg));

    // Set currentDirectory
    currentDirectory = "/";

    // Load FileDescriptors.
    for (Size i = 0; i < FILE_DESCRIPTOR_MAX; i++)
    {
        FileDescriptor fd;
        fd.open = false;

        files.insert(fd);
    }

    // TODO: perhaps we can "bundle" the GetMounts() and ReadProcess() calls, so that
    // we do not need to send IPC message twice in this part (for mounts and getppid())

    // TODO: this is inefficient. It should take only one IPC request to retrieve these things from our parent. Or better, avoid it.

    // Get our parent ID
    ProcessID ppid = getppid();

    // Skip processes with no parent (e.g. from the BootImage)
    if (!ppid)
        return;

    // Inherit file descriptors, current directory, and more.
    // NOTE: we "abuse" the CoreMessage for ipc with our parent...
    IPCMessage(ppid, API::Receive, &msg, sizeof(msg));

    // Copy the file descriptors
    VMCopy(ppid, API::Read, (Address) files.vector(), (Address) msg.path, files.size() * sizeof(FileDescriptor));

    // Dummy reply, to tell our parent we received the fds.... very inefficient.
    IPCMessage(ppid, API::Send, &msg, sizeof(msg));
}

ProcessID findMount(const char *path)
{
    FileSystemMount *m = ZERO;
    Size length = 0, len;
    char tmp[PATHLEN];

    /* Is the path relative? */
    if (path[0] != '/')
    {
        getcwd(tmp, sizeof(tmp));
        snprintf(tmp, sizeof(tmp), "%s/%s", tmp, path);
    }
    else
        strlcpy(tmp, path, PATHLEN);
        
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
    /* All done. */
    return m ? m->procID : ZERO;
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

    /* Clear BSS */
    extern Address __bss_start, __bss_end;
    Address bss_size = &__bss_end - &__bss_start;
    MemoryBlock::set(&__bss_start, 0, bss_size);

    /* Setup the heap, C++ constructors and default mounts. */
    setupHeap();
    runConstructors();
    setupMappings();

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
