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

#include <BootImage.h>
#include <Runtime.h>
#include "CoreServer.h"
#include "CoreMessage.h"
#include <stdio.h>
#include <string.h>

CoreServer::CoreServer()
    : IPCServer<CoreServer, CoreMessage>(this)
{
    SystemInformation info;
    MemoryRange range;
    BootImage *image;
    BootSymbol *symbol;

    /* Register message handlers. */
    addIPCHandler(CreatePrivate,  &CoreServer::createPrivate);
    addIPCHandler(ReservePrivate, &CoreServer::reservePrivate);
    addIPCHandler(ReleasePrivate, &CoreServer::releasePrivate);
    addIPCHandler(SystemMemory,   &CoreServer::systemMemory);
    addIPCHandler(ReadProcess,    &CoreServer::readProcessHandler);
    addIPCHandler(GetMounts,      &CoreServer::getMountsHandler);
    addIPCHandler(SetMount,       &CoreServer::setMountHandler);
    addIPCHandler(ExitProcess,    &CoreServer::exitProcessHandler,  false);
    addIPCHandler(SpawnProcess,   &CoreServer::spawnProcessHandler);
    addIPCHandler(CloneProcess,   &CoreServer::cloneProcessHandler, false);
    addIPCHandler(WaitProcess,    &CoreServer::waitProcessHandler,  false);

    /* Allocate a user process table. */
    procs = new UserProcess[MAX_PROCS];
    memset(procs, 0, sizeof(UserProcess) * MAX_PROCS);

    /* Allocate FileSystemMounts table. */
    mounts = new FileSystemMount[FILESYSTEM_MAXMOUNTS];
    memset(mounts, 0, sizeof(FileSystemMount) * FILESYSTEM_MAXMOUNTS);

    /* We only guarantee that / and /dev, /proc are mounted. */
    strlcpy(mounts[0].path, "/dev", PATHLEN);
    strlcpy(mounts[1].path, "/", PATHLEN);
    strlcpy(mounts[2].path, "/proc", PATHLEN);
    mounts[0].procID  = DEVSRV_PID;
    mounts[0].options = ZERO;
    mounts[1].procID  = ROOTSRV_PID;
    mounts[1].options = ZERO;
    mounts[2].procID  = 13;
    mounts[2].options = ZERO;

    // Attempt to load the boot image
    range.virtualAddress  = findFreeRange(SELF, info.bootImageSize);
    range.physicalAddress = info.bootImageAddress;
    range.access          = Memory::Present | Memory::User | Memory::Readable;
    range.bytes           = info.bootImageSize;
    VMCtl(SELF, Map, &range);
    
    image = (BootImage *) range.virtualAddress;

    /* Loop all embedded programs. */
    for (Size j = 0; j < image->symbolTableCount; j++)
    {
        /* Read out the next program. */
        symbol = (BootSymbol *)(((Address)image) + image->symbolTableOffset);
        symbol += j;

        if (symbol->type != BootProgram)
            continue;

        /* Write commandline. */
        snprintf(procs[j].command, COMMANDLEN,
                "[%s]", symbol->name);

        /* Set user and group identities. */
        procs[j].userID  = 0;
        procs[j].groupID = 0;
    }
}

Address CoreServer::findFreeRange(ProcessID procID, Size size)
{
    Address *pageDir, *pageTab, vaddr, vbegin;

    /* Initialize variables. */
    vbegin  = ZERO;
    vaddr   = 1024 * 1024 * 16;
    pageDir = PAGETABADDR_FROM(PAGETABFROM, PAGEUSERFROM);
    pageTab = PAGETABADDR_FROM(vaddr, PAGEUSERFROM);

    /* Map page tables. */
    VMCtl(procID, MapTables);

    /* Scan tables. */
    for (Size inc = PAGESIZE; DIRENTRY(vaddr) < PAGEDIR_MAX ; vaddr += inc)
    {
	/* Is the hole big enough? */
	if (vbegin && vaddr - vbegin >= size)
	{
	    break;
	}
	/* Increment per page table. */
	inc = PAGETAB_MAX * PAGESIZE;
	
	/* Try the current address. */
	if (pageDir[DIRENTRY(vaddr)] & PAGE_RESERVED)
	{
	    vbegin = ZERO; continue;
	}
	else if (pageDir[DIRENTRY(vaddr)] & PAGE_PRESENT)
	{
	    /* Look further into the page table. */
	    inc     = PAGESIZE;
	    pageTab = PAGETABADDR_FROM(vaddr, PAGEUSERFROM);
	
	    if (pageTab[TABENTRY(vaddr)] & PAGE_PRESENT)
	    {
		vbegin = ZERO; continue;
	    }
	}
	/* Reset start address if needed. */
	if (!vbegin)
	{
	    vbegin = vaddr;
	}
    }
    /* Clean up. */
    VMCtl(procID, UnMapTables);
    
    /* Done. */
    return vbegin;
}

Error CoreServer::insertMapping(ProcessID procID, MemoryRange *range)
{
    MemoryRange tmp;
    Error result;
    
    tmp.virtualAddress = range->virtualAddress;
    tmp.access         = Memory::Present;
    tmp.bytes          = range->bytes;

    /* The given range must be free. */
    if (VMCtl(procID, Access, &tmp))
    {
	return EFAULT;
    }
    /* Perform mapping. */
    if ((result = VMCtl(procID, Map, range)) != 0)
    {
        return result;
    }
    /* Done! */
    return ESUCCESS;
}
