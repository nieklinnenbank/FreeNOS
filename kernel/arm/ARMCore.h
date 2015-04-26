/*
 * Copyright (C) 2015 Niek Linnenbank
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

#ifndef __ARM_CORE_H
#define __ARM_CORE_H

#include <Macros.h>

// TODO: #warning CPUs should be a class too. IntelCPU. ARMCPU. put it in libarch eventually.

/** ARM is little endian */
#define CPU_LITTLE_ENDIAN       1

#include <Types.h>
#include <Macros.h>

/**
 * Retrieve the IRQ number from CPUState.
 * @return IRQ number.
 */
#define IRQ_REG(state) \
    ((state)->vector - 0x20)

/**
 * Reads the CPU's timestamp counter.
 * @return 64-bit integer.
 */
#define timestamp()

/**
 * Reboot the system (by sending the a reset signal on the keyboard I/O port)
 */
#define reboot()

/**
 * Shutdown the machine via ACPI.
 * @note We do not have ACPI yet. Shutdown now has a bit naive implementation.
 * @see http://forum.osdev.org/viewtopic.php?t=16990
 */
#define shutdown()

/**  
 * Puts the CPU in a lower power consuming state. 
 */
#define idle()

/**  
 * Read a byte from a port.  
 * @param port The I/O port to read from.  
 * @return A byte read from the port.  
 */
#define inb(port) \
({ \
    unsigned char b = 0; \
    b; \
})

/**
 * Read a word from a port.
 * @param port The I/O port to read from.
 * @return Word read from the port.
 */
#define inw(port) \
({ \
    unsigned short w = 0; \
    w; \
})

/**  
 * Output a byte to a port.  
 * @param port Port to write to.  
 * @param byte The byte to output.  
 */
#define outb(port,byte)

/**
 * Output a word to a port.
 * @param port Port to write to.
 * @param byte The word to output.
 */
#define outw(port,word)

/** 
 * Output a long to a I/O port. 
 * @param port Target I/O port. 
 * @param l The long 32-bit number to output.
 */
#define outl(port,l)

/** 
 * Contains all the CPU registers. 
 */
typedef struct CPUState
{
}
CPUState;

#endif /* __ARM_CORE_H */
