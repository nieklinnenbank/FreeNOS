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

#ifndef __PCI_PCIDEVICE_H
#define __PCI_PCIDEVICE_H

#include <FreeNOS/CPU.h>
#include <IPCServer.h>
#include <Types.h>
#include <Error.h>

/** Intel's PCI Host controller: address port. */
#define PCI_CONFADDR	0xcf8

/** Intel's PCI Host controller: data port. */
#define PCI_CONFDATA	0xcfc

/** Calculate an address suitable for the PCI_CONFADDR I/O port. */
#define PCI_ADDRESS(bus, dev, func, reg) \
    ((((bus)  << 16) & 0xff0000) | \
     (((dev)  << 11) & 0x00f800) | \
     (((func) << 8)  & 0x000700) | \
      ((reg) & 0xfc) | \
     (0x80000000))
    
/** Read a single byte from PCI configuration space. */
#define PCI_READ_BYTE(bus, dev, func, reg) \
    ({ \
	outl(PCI_CONFADDR, PCI_ADDRESS(bus, dev, func, reg)); \
	inb(PCI_CONFDATA + ((reg) & 3)); \
    })

/** Read a word from PCI configuration space. */
#define PCI_READ_WORD(bus, dev, func, reg) \
    (PCI_READ_BYTE(bus, dev, func, reg) |  \
     PCI_READ_BYTE(bus, dev, func, reg+1) << 8)

/** Read a long from the PCI configuration space. */
#define PCI_READ_LONG(bus, dev, func, reg) \
    (PCI_READ_WORD(bus, dev, func, reg) | \
     PCI_READ_WORD(bus, dev, func, reg+2) << 16)

/** Write a single byte to PCI configuration space. */
#define PCI_WRITE_BYTE(bus, dev, func, reg, val) \
    ({ \
	outl(PCI_CONFADDR, PCI_ADDRESS(bus, dev, func, reg)); \
	outb(PCI_CONFDATA + ((reg) & 3), val); \
    })

/** Write a word to PCI configuration space. */
#define PCI_WRITE_WORD(bus, dev, func, reg, val) \
    (PCI_WRITE_BYTE(bus, dev, func, reg, val), \
     PCI_WRITE_BYTE(bus, dev, func, reg+1, val))

/** Write a long to the PCI configuration space. */
#define PCI_WRITE_LONG(bus, dev, func, reg, val) \
    (PCI_WRITE_WORD(bus, dev, func, reg, val), \
     PCI_WRITE_WORD(bus, dev, func, reg+2, val))

/**
 * PCI configuration structure offsets.
 */
enum
{
    PCI_VID  = 0,
    PCI_DID  = 0x2,
    PCI_CMD  = 0x4,
    PCI_RID  = 0x8,
    PCI_BAR0 = 0x10,
    PCI_BAR1 = 0x14,
    PCI_BAR2 = 0x18,
    PCI_BAR3 = 0x1c,
    PCI_BAR4 = 0x20,
    PCI_BAR5 = 0x24,
    PCI_IRQ  = 0x3c,
};

/**
 * A device detected on the PCI bus.
 */
typedef struct PCIDevice
{
    /**
     * Default constructor.
     */
    PCIDevice() : vendorID(0), deviceID(0), revisionID(0), irq(0),
		  bar0(0), bar1(0), bar2(0), bar3(0), bar4(0), bar5(0)
    {
    }
    
    /**
     * Copy constructor.
     * @param dev PCIDevice instance pointer to copy from.
     */
    PCIDevice(PCIDevice *dev)
    {
	vendorID   = dev->vendorID;
	deviceID   = dev->deviceID;
	revisionID = dev->revisionID;
	irq        = dev->irq;
	bar0       = dev->bar0;
	bar1       = dev->bar1;
	bar2       = dev->bar2;
	bar3       = dev->bar3;
	bar4       = dev->bar4;
	bar5       = dev->bar5;
    }

    /** Identification numbers. */
    u16 vendorID, deviceID, revisionID;

    /** Interrupt Request (IRQ) vector. */
    u16 irq;
    
    /** Base Address Registers (BAR). */
    u8 *bar0, *bar1, *bar2, *bar3, *bar4, *bar5;
}
PCIDevice;

#endif /* __PCI_PCIDEVICE_H */
