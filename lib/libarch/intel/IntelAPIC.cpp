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

#warning Split IntelAPIC in two classes: the interrupt part and timer part. IntelAPICTimer, IntelAPIC

IntelAPIC::IntelAPIC() : IntController()
{
    m_frequency = 0;
    m_int       = TimerVector;
    m_io.setBase(IOBase);
}

IntelIO & IntelAPIC::getIO()
{
    return m_io;
}

uint IntelAPIC::getCounter()
{
    return m_io.read(InitialCount);
}

Timer::Result IntelAPIC::start(IntelPIT *pit)
{
    u32 t1, t2, ic, loops = 20;

    // Start the APIC timer
    m_io.write(DivideConfig, Divide16);
    m_io.write(InitialCount, 0xffffffff);
    m_io.write(Timer, TimerVector | PeriodicMode);

    // Measure the speed of the APIC timer using the
    // known absolute frequency of the PIT timer. First
    // wait for the next PIT trigger.
    pit->waitTrigger();

    // Collect the current APIC timer counter
    t1 = m_io.read(CurrentCount);

    // Wait for several PIT triggers
    for (uint i = 0; i < loops; i++)
        pit->waitTrigger();

    // Measure the current APIC timer counter again.
    t2 = m_io.read(CurrentCount);

    // Configure the APIC timer to run at the same frequency as the PIT.
    ic = (t1 - t2) / loops;
    m_frequency = pit->getFrequency();
    m_io.write(InitialCount, ic);

    // Calculate APIC bus frequency in hertz using the known PIT
    // frequency as reference for diagnostics.
    u32 busFreq = ic * 16 * pit->getFrequency();
    NOTICE("Detected " << busFreq / 1000000 << "."
                       << busFreq % 1000000 << " Mhz APIC bus");
    NOTICE("APIC counter set at " << ic);
    return Timer::Success;
}

Timer::Result IntelAPIC::wait(u32 microseconds)
{
    // TODO: hack: busy wait fixed time.
    for (Size i = 0; i < 10000000; i++)
        microseconds += i;

    if (!m_frequency)
        return microseconds == 0 ? Timer::NotFound : Timer::IOError; // TODO: hack to prevent g++ to optimize the loop away

    Size usecPerInt = 1000000 / m_frequency;
    Size usecPerTick = usecPerInt / m_io.read(InitialCount);
    u32 t1 = m_io.read(CurrentCount), t2;
    u32 waited = 0;

    while (waited < microseconds)
    {
        t2 = m_io.read(CurrentCount);

        if (t2 < t1)
        {
            waited += (t1 - t2) * usecPerTick;
            t1 = t2;
        }
        t1 = t2;
    }
    return Timer::Success;
}

Timer::Result IntelAPIC::start(u32 initialCounter, uint hertz)
{
    // Set hertz
    m_frequency = hertz;

    // Start the APIC timer
    m_io.write(DivideConfig, Divide16);
    m_io.write(InitialCount, initialCounter);
    m_io.write(Timer, TimerVector | PeriodicMode);
    return Timer::Success;
}

Timer::Result IntelAPIC::initialize()
{
    // Map the registers into the address space
    if (m_io.map(IOBase) != IntelIO::Success)
        return Timer::IOError;

#warning TODO: detect the APIC with CPUID
    // if (not detected)
    //     return NotFound;

    // Initialize and disable the timer
    m_io.write(DivideConfig, Divide16);
    m_io.write(InitialCount, 0);
    m_io.write(Timer, TimerVector | PeriodicMode);
    m_io.write(EndOfInterrupt, 0);

    // Enable the APIC
    m_io.set(SpuriousIntVec, APICEnable);
    return Timer::Success;
}

IntController::Result IntelAPIC::enable(uint irq)
{
    return IntController::NotFound;
}

IntController::Result IntelAPIC::disable(uint irq)
{
    return IntController::NotFound;
}

IntController::Result IntelAPIC::clear(uint irq)
{
    m_io.write(EndOfInterrupt, 0);
    return IntController::Success;
}

IntController::Result IntelAPIC::sendStartupIPI(uint cpuId, Address addr)
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

    // Wait 10 miliseconds
    wait(10000);

    // Send two SIPI's
    for (Size i = 0; i < 2; i++)
    {
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

        // Wait 1 milisecond
#warning FIX this.
        // TODO: this is difficult in the current implementation, because
        //       the *userspace* instance of this class does not have
        //       m_frequency set.. better solution is that libarch provides a Timer
        //       abstract class, and that class should provide a consistent interface
        //       for (busy) waiting on timer events, triggering, etc, where it hides
        //       if the call is done with a kernel trap or direct register write etc.
        wait(1000);
    }

    // Startup interrupt delivered.
    return IntController::Success;
}
