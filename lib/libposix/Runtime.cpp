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

#include <API/ProcessCtl.h>
#include <Arch/Memory.h>
#include <Types.h>
#include <Macros.h>
#include <PageAllocator.h>
#include <PoolAllocator.h>
#include <VMCtlAllocator.h>
#include <ProcessServer.h>
#include <stdlib.h>
#include "Runtime.h"
#include <string.h>
#include "unistd.h"

Shared<FileSystemMount> mounts;
Shared<FileDescriptor> files;
Shared<UserProcess> procs;

/** List of constructors. */
extern void (*CTOR_LIST)();

/** List of destructors. */
extern void (*DTOR_LIST)();

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

    /* Only the memory server allocates directly. */
    if (ProcessCtl(SELF, GetPID) == MEMSRV_PID)
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
    char key[256];

    if (getpid() != MEMSRV_PID)
    {
        /* Load the mounts and process table. */
        mounts.load(FILE_SYSTEM_MOUNT_KEY, MAX_MOUNTS);
        procs.load(USER_PROCESS_KEY, MAX_PROCS);
    
        /* Format FileDescriptor key. */
        snprintf(key, sizeof(key), "%s%u", FILE_DESCRIPTOR_KEY, getpid());
    
        /* Then load the FileDescriptor table. */
        files.load(key, FILE_DESCRIPTOR_MAX);
    }
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
    for (Size i = 0; i < MAX_MOUNTS; i++)
    {
        if (mounts[i]->path[0])
        {
            len = strlen(mounts[i]->path);
    
            /*
             * Only choose this mount, if it matches,
             * and is longer than the last match.
             */
            if (strncmp(tmp, mounts[i]->path, len) == 0 && len > length)
            {
                length = len;
                m = mounts[i];
            }
        }
    }
    /* All done. */
    return m ? m->procID : ZERO;
}

ProcessID findMount(int fildes)
{
    return files[fildes] ? files[fildes]->mount : ZERO;
}

Shared<FileSystemMount> * getMounts()
{
    return &mounts;
}

Shared<UserProcess> * getProcesses()
{
    return &procs;
}

Shared<FileDescriptor> * getFiles()
{
    return &files;
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
