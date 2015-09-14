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

#include <Log.h>
#include <MemoryContext.h>
#include "IntelPIT.h"
#include "IntelAPIC.h"

IntelAPIC::IntelAPIC() : IntController()
{
    m_hertz = 0;
    m_io.setBase(IOBase);
}

IntelIO & IntelAPIC::getIO()
{
    return m_io;
}

uint IntelAPIC::getTimerInterrupt()
{
    return TimerVector;
}

uint IntelAPIC::getTimerFrequency()
{
    return m_hertz;
}

uint IntelAPIC::getTimerCounter()
{
    return m_io.read(InitialCount);
}

IntelAPIC::Result IntelAPIC::startTimer(IntelPIT *pit)
{
    u32 t1, t2, ic, loops = 20;

    // Start the APIC timer
    m_io.write(DivideConfig, Divide16);
    m_io.write(InitialCount, 0xffffffff);
    m_io.write(Timer, TimerVector | PeriodicMode);

    // Measure the speed of the APIC timer using the
    // known absolute frequency of the PIT timer. First
    // wait for the next PIT trigger.
    pit->wait();

    // Collect the current APIC timer counter
    t1 = m_io.read(CurrentCount);

    // Wait for several PIT triggers
    for (uint i = 0; i < loops; i++)
        pit->wait();

    // Measure the current APIC timer counter again.
    t2 = m_io.read(CurrentCount);

    // Configure the APIC timer to run at the same frequency as the PIT.
    ic = (t1 - t2) / loops;
    m_hertz = pit->getFrequency();
    m_io.write(InitialCount, ic);

    // Calculate APIC bus frequency in hertz using the known PIT
    // frequency as reference for diagnostics.
    u32 busFreq = ic * 16 * pit->getFrequency();
    NOTICE("Detected " << busFreq / 1000000 << "."
                       << busFreq % 1000000 << " Mhz APIC bus");
    NOTICE("APIC counter set at " << ic);
    return Success;
}

IntelAPIC::Result IntelAPIC::startTimer(u32 initialCounter, uint hertz)
{
    // Set hertz
    m_hertz = hertz;

    // Start the APIC timer
    m_io.write(DivideConfig, Divide16);
    m_io.write(InitialCount, initialCounter);
    m_io.write(Timer, TimerVector | PeriodicMode);
    return Success;
}

IntelAPIC::Result IntelAPIC::initialize()
{
    // Map the registers into the address space
    if (m_io.map(IOBase) != IntelIO::Success)
        return IOError;

    // TODO: detect the APIC with CPUID
    // if (not detected)
    //     return NotFound;

    // Initialize and disable the timer
    m_io.write(DivideConfig, Divide16);
    m_io.write(InitialCount, 0);
    m_io.write(Timer, TimerVector | PeriodicMode);
    m_io.write(EndOfInterrupt, 0);

    // Enable the APIC
    m_io.set(SpuriousIntVec, APICEnable);
    return Success;
}

IntelAPIC::Result IntelAPIC::enable(uint irq)
{
    return NotFound;
}

IntelAPIC::Result IntelAPIC::disable(uint irq)
{
    return NotFound;
}

IntelAPIC::Result IntelAPIC::clear(uint irq)
{
    m_io.write(EndOfInterrupt, 0);
    return Success;
}

IntelAPIC::Result IntelAPIC::sendStartupIPI(uint cpuId, Address addr)
{
    ulong cfg;

    // Write APIC Destination
    cfg  = m_io.read(IntCommand2);
    cfg &= 0x00ffffff;
    m_io.write(IntCommand2, cfg | APIC_DEST(cpuId));

    // Assert INIT
    cfg  = m_io.read(IntCommand1);
    cfg &= ~0xcdfff;
    cfg |= (APIC_DEST_FIELD | APIC_DEST_LEVELTRIG |
            APIC_DEST_ASSERT | APIC_DEST_DM_INIT);
    m_io.write(IntCommand1, cfg);

    // Write APIC Destination
    cfg  = m_io.read(IntCommand2);
    cfg &= 0x00ffffff;
    m_io.write(IntCommand2, cfg | APIC_DEST(cpuId));

    // Assert STARTUP
    cfg  = m_io.read(IntCommand1);
    cfg &= ~0xcdfff;
    cfg |= (APIC_DEST_FIELD | APIC_DEST_DM_STARTUP |
           (addr >> 12));
    m_io.write(IntCommand1, cfg);

    // Startup interrupt delivered.
    return Success;
}
