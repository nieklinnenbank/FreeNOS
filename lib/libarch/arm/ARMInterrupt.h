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

#ifndef __LIBARCH_ARM_INTERRUPT_H
#define __LIBARCH_ARM_INTERRUPT_H

#include <Types.h>
#include <IntController.h>

/** Size of the ARM exception vector table in bytes */
#define ARM_INTR_VECTAB_SIZE   (64)

/**
 * ARM interrupt controller.
 *
 * This class provides generic functionity for installing
 * and servicing interrupts on ARM. Each ARM system should
 * provide a ARM interrupt controller implementation which
 * inherits from this class.
 *
 * @see http://www.state-machine.com/arm/Building_bare-metal_ARM_with_GNU.pdf
 */
class ARMInterrupt : public IntController
{
  public:

    /**
     * ARM interrupt types.
     */
    enum Interrupt
    {
        Reset = 0,
        UndefinedInstruction,
        SoftwareInterrupt,
        PrefetchAbort,
        DataAbort,
        Reserved,
        IRQ,
        FIQ
    };

    /**
     * Constructor
     *
     * @param base Base address of the interrupt jump table.
     *             By default this is physical address 0x0 in RAM.
     */
    ARMInterrupt(Address base = 0x0);

    /**
     * Install an interrupt handler.
     *
     * @param handler Interrupt handler function to install.
     */
    Result install(Interrupt vector, Handler handler);

    /**
     * Check if an IRQ vector is set.
     */
    virtual bool isTriggered(uint irq) = 0;

  private:

    /**
     * ARM interrupt jump table.
     *
     * The ARM processor will start executing at the appropriate
     * vector offset in the jump table and perform a jump to
     * the interrupt handler address, which it retrieves from the 
     * interrupt handlers table.
     *
     * @see m_handlerTable;
     * @see ARMInterruptTable.S
     */
    Address m_vecTable;
};

#endif /* __LIBARCH_ARM_INTERRUPT_H */
