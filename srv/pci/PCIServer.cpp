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
#include <PseudoFile.h>
#include "PCIServer.h"
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    if (!fork())
    {
	/* (Re)create target directory. */
        mkdir("/dev/pci", S_IWUSR | S_IRUSR);

	/*
	 * Start serving requests.
	 */
	PCIServer server("/dev/pci");
	return server.run();
    }
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
	syslog(LOG_INFO, "no host controller found");
	exit(EXIT_FAILURE);
    }
    else
    {
	syslog(LOG_INFO, "host controller found");
	scan();
    }
}

void PCIServer::scan()
{
    u16 vendorID, deviceID, rev, irq;
    u32 bar0, bar1, bar2, bar3, bar4, bar5;    
    Directory *busDir = ZERO, *slotDir = ZERO;

    /* Scan the PCI bus. */
    for (u16 bus = 0; bus < 0xff; bus++)
    {
	for (u16 slot = 0; slot < 0x8; slot++)
        {
	    /* Read device ID's. */
    	    vendorID = PCI_READ_WORD(bus, slot, 0, PCI_VID);
    	    deviceID = PCI_READ_WORD(bus, slot, 0, PCI_DID);

	    /* Is this a valid device? */
    	    if (vendorID != 0xffff && deviceID != 0xffff)
    	    {
		/* Read out configuration. */
		rev  = PCI_READ_BYTE(bus, slot, 0, PCI_RID);
		irq  = PCI_READ_BYTE(bus, slot, 0, PCI_IRQ);
		bar0 = PCI_READ_LONG(bus, slot, 0, PCI_BAR0);
		bar1 = PCI_READ_LONG(bus, slot, 0, PCI_BAR1);
		bar2 = PCI_READ_LONG(bus, slot, 0, PCI_BAR2);
		bar3 = PCI_READ_LONG(bus, slot, 0, PCI_BAR3);
		bar4 = PCI_READ_LONG(bus, slot, 0, PCI_BAR4);
		bar5 = PCI_READ_LONG(bus, slot, 0, PCI_BAR5);

		/* Create bus directory, if needed. */
		if (!busDir)
		{
		    busDir = new Directory;
		    rootDir->insert(DirectoryFile, "%x", bus);
		    insertFileCache(busDir, "%x", bus);
		}
		/* Create slot directory. */
		slotDir = new Directory;
		slotDir->insert(DirectoryFile, ".");
		slotDir->insert(DirectoryFile, "..");
		slotDir->insert(RegularFile, "vendor");
		slotDir->insert(RegularFile, "device");
		slotDir->insert(RegularFile, "revision");
		slotDir->insert(RegularFile, "interrupt");
		slotDir->insert(RegularFile, "bar0");
		slotDir->insert(RegularFile, "bar1");
		slotDir->insert(RegularFile, "bar2");
		slotDir->insert(RegularFile, "bar3");
		slotDir->insert(RegularFile, "bar4");
		slotDir->insert(RegularFile, "bar5");

		/* Add the slot directory to the bus directory. */
		busDir->insert(DirectoryFile, "%x", slot);
		
    	        /* Insert into the cache. */
	        insertFileCache(slotDir, "%x/%x", bus, slot);
		insertFileCache(new PseudoFile("%x", vendorID),
				"%x/%x/vendor", bus, slot);
		insertFileCache(new PseudoFile("%x", deviceID),
				"%x/%x/device", bus, slot);
		insertFileCache(new PseudoFile("%u", rev),
				"%x/%x/revision",  bus, slot);
		insertFileCache(new PseudoFile("%u", irq),
				"%x/%x/interrupt", bus, slot);
		insertFileCache(new PseudoFile("%x", bar0),
				"%x/%x/bar0", bus, slot);
		insertFileCache(new PseudoFile("%x", bar1),
				"%x/%x/bar1", bus, slot);
		insertFileCache(new PseudoFile("%x", bar2),
				"%x/%x/bar2", bus, slot);
		insertFileCache(new PseudoFile("%x", bar3),
				"%x/%x/bar3", bus, slot);
		insertFileCache(new PseudoFile("%x", bar4),
				"%x/%x/bar4", bus, slot);
		insertFileCache(new PseudoFile("%x", bar5),
				"%x/%x/bar5", bus, slot);

	        /* Log the device. */
		syslog(LOG_INFO, "[%x:%x] %x:%x (rev %d)",
    		       bus, slot, vendorID, deviceID, rev);
    	    }
	    slotDir = ZERO;
	}
	busDir  = ZERO;
    }
}

void PCIServer::runDeviceServer(u16 vendorID, u16 deviceID)
{
    ProcessID pid;
    struct stat st;
    char path[PATHLEN];
    const char *args[4];
    int status;

    /*
     * Attempt to execute an external driver program
     * to handle I/O for the PCI device. First we construct
     * the full path to the handler, if any.
     */
    snprintf(path, sizeof(path), "/srv/pci/%x/%x",
	     vendorID, deviceID);

    /* If there is no device server, forget it. */
    if (stat(path, &st) == -1)
    {
	return;
    }

    /*
     * Construct an argument list.
     */
    for (int i = 0; i < 3; i++)
    {
	args[i] = new char[PATHLEN];
    }
    args[3] = ZERO;
    snprintf((char*)args[0], PATHLEN, "%s", path);
    snprintf((char*)args[1], PATHLEN, "%x", vendorID);
    snprintf((char*)args[2], PATHLEN, "%x", deviceID);

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
