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

#ifndef __PCI_PCISERVER_H
#define __PCI_PCISERVER_H

#include <FreeNOS/System.h>
#include <FileSystem.h>
#include <Directory.h>
#include <Types.h>

/**                                                                                                                                                                                                      
 * @defgroup pci PCI (Peripheral Component Interconnect) 
 * @{ 
 */

/**
 * @name PCI I/O ports.
 * @{
 */

/** @brief Intel's PCI Host controller: address port. */
#define PCI_CONFADDR    0xcf8

/** @brief Intel's PCI Host controller: data port. */
#define PCI_CONFDATA    0xcfc

/**
 * @}
 */

/**
 * @name PCI Registers
 * @{
 */

/** @brief Vendor Identity. */
#define PCI_VID  0x0

/** @brief Device Identity. */
#define PCI_DID  0x2

/** @brief Command. */
#define PCI_CMD  0x4

/** @brief Revision Identity. */
#define PCI_RID  0x8

/** @brief Base Address Register 0. */
#define PCI_BAR0 0x10

/** @brief Base Address Register 1. */
#define PCI_BAR1 0x14

/** @brief Base Address Register 2. */
#define PCI_BAR2 0x18

/** @brief Base Address Register 3. */
#define PCI_BAR3 0x1c

/** @brief Base Address Register 4. */
#define PCI_BAR4 0x20

/** @brief Base Address Register 5. */
#define PCI_BAR5 0x24

/** @brief Interrupt Request Vector. */
#define PCI_IRQ  0x3c

/**
 * @}
 */

/**
 * @name PCI Macros
 * @{
 */

/**
 * @brief Calculate an address suitable for the PCI_CONFADDR I/O port.
 */
#define PCI_ADDRESS(bus, dev, func, reg) \
    ((((bus)  << 16) & 0xff0000) | \
     (((dev)  << 11) & 0x00f800) | \
     (((func) << 8)  & 0x000700) | \
      ((reg) & 0xfc) | \
     (0x80000000))
    
/**
 * @brief Read a single byte from PCI configuration space.
 */
#define PCI_READ_BYTE(bus, dev, func, reg) \
    ({ \
    WriteLong(PCI_CONFADDR, PCI_ADDRESS(bus, dev, func, reg)); \
    ReadByte(PCI_CONFDATA + ((reg) & 3)); \
    })

/**
 * @brief Read a word from PCI configuration space.
 */
#define PCI_READ_WORD(bus, dev, func, reg) \
    (PCI_READ_BYTE(bus, dev, func, reg) |  \
     PCI_READ_BYTE(bus, dev, func, reg+1) << 8)

/**
 * @brief Read a long from the PCI configuration space. */
#define PCI_READ_LONG(bus, dev, func, reg) \
    (PCI_READ_WORD(bus, dev, func, reg) | \
     PCI_READ_WORD(bus, dev, func, reg+2) << 16)

/**
 * @brief Write a single byte to PCI configuration space.
 */
#define PCI_WRITE_BYTE(bus, dev, func, reg, val) \
    ({ \
    WriteLong(PCI_CONFADDR, PCI_ADDRESS(bus, dev, func, reg)); \
    WriteByte(PCI_CONFDATA + ((reg) & 3), val); \
    })

/**
 * @brief Write a word to PCI configuration space.
 */
#define PCI_WRITE_WORD(bus, dev, func, reg, val) \
    (PCI_WRITE_BYTE(bus, dev, func, reg, val), \
     PCI_WRITE_BYTE(bus, dev, func, reg+1, val))

/**
 * @brief Write a long to the PCI configuration space.
 */
#define PCI_WRITE_LONG(bus, dev, func, reg, val) \
    (PCI_WRITE_WORD(bus, dev, func, reg, val), \
     PCI_WRITE_WORD(bus, dev, func, reg+2, val))

/**
 * @}
 */

/**
 * @brief Peripheral Component Interface FileSystem server.
 *
 * This program maps registers from the PCI configuration
 * space to regular (pseudo)files. To access the PCI bus
 * from a user program, simply use the files in /dev/pci/$bus/$slot/$func.
 *
 * @see FileSystem
 */
class PCIServer : public FileSystem
{
    public:
    
    /** 
         * Class constructor function. 
         * @param path Path to which we are mounted. 
         */
        PCIServer(const char *path);
    
    /**
     * @brief (Re)scans the PCI bus.
     */
    void scan();
    
    private:

    /**
     * @brief Create a PCI slot directory.
     *
     * The created directory will contain a (pseudo)file
     * for PCI configuration registers available via the PCI
     * configuration space.
     *
     * @param bus PCI Bus number.
     * @param slot Slot number in the PCI bus.
     * @param func Device function number in the slot.
     */
    void detect(u16 bus, u16 slot, u16 func);

    /** @brief Root directory pointer. */
    Directory *rootDir;
};

/**
 * @}
 */

#endif /* __PCI_PCISERVER_H */
