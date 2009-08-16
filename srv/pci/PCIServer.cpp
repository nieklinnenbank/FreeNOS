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
#include <Config.h>
#include "PCIFile.h"
#include "PCIServer.h"
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    /* (Re)create target directory. */
    mkdir("/dev/pci", S_IWUSR | S_IRUSR);

    /* Create instance. */
    PCIServer server("/dev/pci");

    /* Start serving requests. */
    if (!fork())
    {
	/*
	 * TODO: Please see issue 76:
	 *  http://code.google.com/p/freenos/issues/detail?id=76
	 */
        for (int i = 0; i < 4; i++)
	{
	    ProcessCtl(SELF, AllowIO, PCI_CONFADDR + i);
	    ProcessCtl(SELF, AllowIO, PCI_CONFDATA + i);
	}
	return server.run();
    }
    return EXIT_FAILURE;
}

PCIServer::PCIServer(const char *path)
    : FileSystem(path), rootDir(new Directory)
{
    /* Set a root directory. */
    setRoot(rootDir);

    /* Open the system log. */
    openlog("PCI", LOG_PID, LOG_USER);

    /* Request I/O ports. */
    for (int i = 0; i < 4; i++)
    {
	ProcessCtl(SELF, AllowIO, PCI_CONFADDR + i);
	ProcessCtl(SELF, AllowIO, PCI_CONFDATA + i);
    }
    /* Detect PCI host controller. */
    if (PCI_READ_WORD(0, 0, 0, PCI_VID) == 0xffff ||
        PCI_READ_WORD(0, 0, 0, PCI_DID) == 0xffff)
    {
	syslog(LOG_INFO, "No Host Controller found");
	exit(EXIT_FAILURE);
    }
    else
    {
	syslog(LOG_INFO, "Host Controller found");
	scan();
    }
}

void PCIServer::scan()
{
    Directory *busDir = ZERO, *slotDir = ZERO;
    u16 vendorID, deviceID, revisionID;
    
    /*
     * Walk the PCI bus by performing a read
     * on the vendor and device ID's for each possible
     * bus, slot and function combination.
     */
    for (u16 bus = 0; bus < 256; bus++)
    {
	for (u16 slot = 0; slot < 32; slot++)
	{
	    for (u16 func = 0; func < 8; func++)
	    {
	        /* Read ID's. */
	        vendorID   = PCI_READ_WORD(bus, slot, func, PCI_VID);
	        deviceID   = PCI_READ_WORD(bus, slot, func, PCI_DID);
		revisionID = PCI_READ_BYTE(bus, slot, func, PCI_RID);

	        /* Is this a valid device? */
	        if (vendorID == 0xffff || deviceID == 0xffff)
	        {
		    continue;
	        }
	        /* Create bus directory, if needed. */
	        if (!busDir)
	        {
		    busDir = new Directory;
		    rootDir->insert(DirectoryFile, "%x", bus);
		    insertFileCache(busDir, "%x", bus);
		}
		/* Make slot directory, if needed. */
		if (!slotDir)
		{
		    slotDir = new Directory;

		    busDir->insert(DirectoryFile, "%x", slot);
	    	    insertFileCache(slotDir, "%x/%x", bus, slot);
		}
		/* Then make & fill the function directory. */
		detect(bus, slot, func);
		slotDir->insert(DirectoryFile, "%x", func);

	        /* Log the device. */
		syslog(LOG_INFO, "[%x:%x:%x] %x:%x (rev %d)",
		       bus, slot, func, vendorID, deviceID, revisionID);
		       
		/* Execute a device server. */
		runDeviceServer(bus, slot, func, vendorID, deviceID);
	    }
	    slotDir = ZERO;
	}
	busDir = ZERO;
    }
}

void PCIServer::detect(u16 bus, u16 slot, u16 func)
{
    Directory *dir;

    /* Create PCI function directory. Fill it with entries. */
    dir = new Directory;
    dir->insert(DirectoryFile, ".");
    dir->insert(DirectoryFile, "..");
    dir->insert(RegularFile, "vendor");
    dir->insert(RegularFile, "device");
    dir->insert(RegularFile, "revision");
    dir->insert(RegularFile, "interrupt");
    dir->insert(RegularFile, "bar0");
    dir->insert(RegularFile, "bar1");
    dir->insert(RegularFile, "bar2");
    dir->insert(RegularFile, "bar3");
    dir->insert(RegularFile, "bar4");
    dir->insert(RegularFile, "bar5");
    insertFileCache(dir, "%x/%x/%x", bus, slot, func);

    /*
     * Now create actual files.
     * Put them into the cache.
     */
    insertFileCache(new PCIFile(bus, slot, func, PCI_VID, 2),
		    "%x/%x/%x/vendor", bus, slot, func);
	
    insertFileCache(new PCIFile(bus, slot, func, PCI_DID, 2),
		    "%x/%x/%x/device", bus, slot, func);
				
    insertFileCache(new PCIFile(bus, slot, func, PCI_RID, 1),
		    "%x/%x/%x/revision",  bus, slot, func);

    insertFileCache(new PCIFile(bus, slot, func, PCI_IRQ, 1),
		    "%x/%x/%x/interrupt", bus, slot, func);

    insertFileCache(new PCIFile(bus, slot, func, PCI_BAR0, 4),
		    "%x/%x/%x/bar0", bus, slot, func);

    insertFileCache(new PCIFile(bus, slot, func, PCI_BAR1, 4),
		    "%x/%x/%x/bar1", bus, slot, func);

    insertFileCache(new PCIFile(bus, slot, func, PCI_BAR2, 4),
		    "%x/%x/%x/bar2", bus, slot, func);

    insertFileCache(new PCIFile(bus, slot, func, PCI_BAR3, 4),
		    "%x/%x/%x/bar3", bus, slot, func);

    insertFileCache(new PCIFile(bus, slot, func, PCI_BAR4, 4),
		    "%x/%x/%x/bar4", bus, slot, func);

    insertFileCache(new PCIFile(bus, slot, func, PCI_BAR5, 4),
		    "%x/%x/%x/bar5", bus, slot, func);
}

void PCIServer::runDeviceServer(u16 bus, u16 slot, u16 func,
				u16 vendorID, u16 deviceID)
{
    ProcessID pid;
    struct stat st;
    char path[PATHLEN];
    const char *args[5];
    int status;

    /*
     * Attempt to execute an external driver program
     * to handle I/O for the PCI device. First we construct
     * the full path to the handler, if any.
     */
    snprintf(path, sizeof(path), "/etc/pci/%x:%x",
	     vendorID, deviceID);

    /* If there is no device server, forget it. */
    if (stat(path, &st) == -1)
    {
	return;
    }

    /*
     * Construct an argument list.
     */
    for (int i = 0; i < 4; i++)
    {
	args[i] = new char[PATHLEN];
    }
    args[4] = ZERO;
    snprintf((char*)args[0], PATHLEN, "%s", path);
    snprintf((char*)args[1], PATHLEN, "%x", bus);
    snprintf((char*)args[2], PATHLEN, "%x", slot);
    snprintf((char*)args[3], PATHLEN, "%x", func);

    /*
     * Try to fork off a child for the device server.
     */
    if ((pid = forkexec(path, args)) == (pid_t) -1)
    {
	syslog(LOG_ERR, "failed to forkexec() `%s': %s\r\n",
			 path, strerror(errno));
    }
    else
	waitpid(pid, &status, 0);
}
