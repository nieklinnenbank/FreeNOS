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
#include <FreeNOS/System/Constant.h>
#include <Types.h>
#include <Macros.h>
#include <Array.h>
#include <PageAllocator.h>
#include <PoolAllocator.h>
#include <VMCtlAllocator.h>
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

/** Array of FileDescriptors. */
Array<FileDescriptor, FILE_DESCRIPTOR_MAX> *files = ZERO;

/** Current Directory String */
String *currentDirectory = ZERO;

extern C int __cxa_atexit(void (*func) (void *),
                          void * arg, void * dso_handle)
{
    return (0);
}

extern C void __cxa_pure_virtual()
{
}

extern C void __dso_handle()
{
}

extern C void __stack_chk_fail(void)
{
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
    Address heapAddr, heapOff;
    Size parentSize;

    /* Only the core server allocates directly. */
    if (ProcessCtl(SELF, GetPID) == CORESRV_PID)
    {
        VMCtlAllocator alloc(PAGESIZE * 4);

        /* Allocate instance copy on vm pages itself. */
        heapAddr   = alloc.getHeapStart();
        parent     = new (heapAddr) VMCtlAllocator(&alloc);
        parentSize = sizeof(VMCtlAllocator);
    }
    else
    {
        PageAllocator alloc(PAGESIZE * 4);

        /* Allocate instance copy on vm pages itself. */
        heapAddr   = alloc.getStart();
        parent     = new (heapAddr) PageAllocator(&alloc);
        parentSize = sizeof(PageAllocator);
    }
    /* Make a pool. */
    pool = new (heapAddr + parentSize) PoolAllocator();
    
    /* Point to the next free space. */
    heapOff   = parentSize + sizeof(PoolAllocator);
    heapAddr += heapOff;

    /* Setup the userspace heap allocator region. */
    pool->region(heapAddr, (PAGESIZE * 4) - heapOff);
    pool->setParent(parent);

    /* Set default allocator. */
    Allocator::setDefault(pool);
}

void setupMappings()
{
    // The CoreServer does not need to setup mappings
    if (getpid() == CORESRV_PID)
        return;

    // TODO: Ask CoreServer for FileSystemMounts table
    memset(&mounts, 0, sizeof(mounts));
    strcpy(mounts[0].path, "/");
    strcpy(mounts[1].path, "/dev");
    strcpy(mounts[2].path, "/proc");
    mounts[0].procID = ROOTSRV_PID;
    mounts[1].procID = DEVSRV_PID;
    mounts[2].procID = 13;

    // Set currentDirectory
    currentDirectory = new String("/");

    // Load FileDescriptors
    files = new Array<FileDescriptor, FILE_DESCRIPTOR_MAX>();

    // Mark all FileDescriptors closed
    for (uint i = 0; i < files->size(); i++)
        (*files)[i].open = false;

    // TODO: we need to check if we have a parent, and if so, inherit some things, like
    // the filedescriptors and currentDirectory..

    // TODO: temporary hardcode standard I/O file descriptors to /dev/tty0 (procID 11)
    (*files)[0].open = true;
    (*files)[0].path = new String("/dev/tty0");
    (*files)[0].mount = 11;
    (*files)[1].open = true;
    (*files)[1].path = new String("/dev/tty0");
    (*files)[1].mount = 11;
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
    return files->get(fildes) ? files->get(fildes)->mount : ZERO;
}

FileSystemMount * getMounts()
{
    return mounts;
}

Array<FileDescriptor, FILE_DESCRIPTOR_MAX> * getFiles()
{
    return files;
}

String * getCurrentDirectory()
{
    return currentDirectory;
}

extern C void SECTION(".entry") _entry() 
{
    int ret, argc;
    char *arguments;
    char **argv;

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
