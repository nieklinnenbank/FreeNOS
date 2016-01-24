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

#ifndef __LIBARCH_ARM_BROADCOMINTERRUPT_H
#define __LIBARCH_ARM_BROADCOMINTERRUPT_H

#include <arm/ARMInterrupt.h>
#include <arm/ARMIO.h>

#define INTERRUPT_BASE_ADDR        0xB000
#define INTERRUPT_BASICPEND       (INTERRUPT_BASE_ADDR+0x200)
#define INTERRUPT_IRQPEND1        (INTERRUPT_BASE_ADDR+0x204)
#define INTERRUPT_IRQPEND2        (INTERRUPT_BASE_ADDR+0x208)
#define INTERRUPT_FIQCONTROL      (INTERRUPT_BASE_ADDR+0x20C)
#define INTERRUPT_ENABLEIRQ1      (INTERRUPT_BASE_ADDR+0x210)
#define INTERRUPT_ENABLEIRQ2      (INTERRUPT_BASE_ADDR+0x214)
#define INTERRUPT_ENABLEBASICIRQ  (INTERRUPT_BASE_ADDR+0x218)
#define INTERRUPT_DISABLEIRQ1     (INTERRUPT_BASE_ADDR+0x21C)
#define INTERRUPT_DISABLEIRQ2     (INTERRUPT_BASE_ADDR+0x220)
#define INTERRUPT_DISABLEBASICIRQ (INTERRUPT_BASE_ADDR+0x224)
   #define IRQSYSTIMERC1 1
   #define IRQSYSTIMERC3 3
   #define IRQAUX        29
   #define IRQUART       57

//see the table page 113 of the BCM2835 Arm Peripherals datasheet for vector numbers
//triggered when the system timer matches the C1 register
#define BCM_IRQ_SYSTIMERM1 1  
//triggered when the PL011 asserts IRQ status
#define BCM_IRQ_PL011      57 

/**
 * Raspberry Pi Interrupt Controller implementation
 */
class BroadcomInterrupt : public ARMInterrupt
{
  public:

    /**
     * Constructor
     */
    BroadcomInterrupt();

    /**
     * Enable an IRQ vector
     */
    virtual Result enable(uint vector);

    /**
     * Disable an IRQ vector
     */
    virtual Result disable(uint vector);

    /**
     * Clear an IRQ vector
     */
    virtual Result clear(uint vector);

    /**
     * Check if an IRQ vector is set.
     */
    virtual bool isTriggered(uint vector);

  private:

    /** I/O instance */
    ARMIO m_io;
};

#endif /* LIBARCH_ARM_BROADCOMINTERRUPT_H */
