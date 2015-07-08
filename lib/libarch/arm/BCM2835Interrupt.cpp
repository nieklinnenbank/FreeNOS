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

#include "BCM2835Interrupt.h"

BCM2835Interrupt::BCM2835Interrupt() : ARMInterrupt()
{
    //disable all IRQ sources first, just to be "safe"
    INTERRUPT_DISABLEIRQ1 = 0xFFFFFFFF;
    INTERRUPT_DISABLEIRQ2 = 0xFFFFFFFF;
}

void BCM2835Interrupt::enableIRQ(u32 vector)
{
    //enable the respective interrupt
    if(vector < 32)
    {
        //only 1 bits are recognized when writing to the (en/dis)able regs.
        //  using |= here could be problematic since it would likely be
        //  implemented as multiple instructions: at least a read, an or,
        //  and a write. if we interrupted _after_ the read instruction or
        //  the or instruction, and disabled certain bits in the IRQ
        //  routine, the |= would write back the old state of the enable
        //  bits. This would effectively be re-enabling interrupts that we
        //  wanted disabled.
        INTERRUPT_ENABLEIRQ1 = (1<<vector); //zeroes are ignored, don't use |=
    }
    else
    {
        INTERRUPT_ENABLEIRQ2 = (1<<(vector-32)); //zeroes are ignored, don't use |=
    }
}

void BCM2835Interrupt::disableIRQ(u32 vector)
{
    //disable IRQs for this device before NULL-ing the vector. otherwise,
    //  we might interrupt with a NULL_VECT in the handler's address.
    //  because the interrupt wasn't ACKed because we never went to the
    //  handler routine, the device will continue to assert its IRQ line,
    //  which will put us in a never-ending IRQ loop.
    if(vector < 32)
    {
        INTERRUPT_DISABLEIRQ1 = (1<<vector); //zeroes are ignored, don't use |=
    }
    else
    {
        INTERRUPT_DISABLEIRQ2 = (1<<(vector-32)); //zeroes are ignored, don't use |=
    }
}

bool BCM2835Interrupt::isTriggered(u32 vector)
{
    u32 pend1 = INTERRUPT_IRQPEND1;
    u32 pend2 = INTERRUPT_IRQPEND2;

    if (vector < 32)
        return (pend1 & (1 << vector));
    else
        return (pend2 & (1 << vector));
}
