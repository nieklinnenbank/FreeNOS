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

#ifndef __LIBARCH_INTEL_APIC_H
#define __LIBARCH_INTEL_APIC_H

#include <Types.h>
#include <BitOperations.h>
#include <IntController.h>
#include <Timer.h>
#include "IntelIO.h"

/** Forward declarations */
class MemoryContext;
class IntelPIT;

// TODO (needed for SMP startup). Move inside class as members.
#define APIC_DEST(x) ((x) << 24)
#define APIC_DEST_FIELD         0x00000
#define APIC_DEST_LEVELTRIG     0x08000
#define APIC_DEST_ASSERT        0x04000
#define APIC_DEST_DM_INIT       0x00500
#define APIC_DEST_DM_STARTUP    0x00600

/**
 * Intel Advanced Programmable Interrupt Controller (APIC)
 */
class IntelAPIC : public IntController, public Timer
{
  public:

    /** APIC memory mapped I/O register base offset (physical address). */
    static const uint IOBase = 0xfee00000;

    /** APIC timer interrupt vector is fixed at 48 */
    static const uint TimerVector = 48;

  private:

    /**
     * Hardware registers.
     */
    enum Registers
    {
        Identifier          = 0x20,
        Version             = 0x30,
        TaskPriority        = 0x80,
        ArbitrationPriority = 0x90,
        ProcessorPriority   = 0xa0,
        EndOfInterrupt      = 0xb0,
        SpuriousIntVec      = 0xf0,
        InService           = 0x100,
        TriggerMode         = 0x180,
        IntRequest          = 0x200,
        ErrorStatus         = 0x280,
        IntCommand1         = 0x300,
        IntCommand2         = 0x310,
        Timer               = 0x320,
        ThermalSensor       = 0x330,
        PerfCounters        = 0x340,
        LocalInt0           = 0x350,
        LocalInt1           = 0x360,
        Error               = 0x370,
        InitialCount        = 0x380,
        CurrentCount        = 0x390,
        DivideConfig        = 0x3e0
    };

    /**
     * Spurious Interrupt Vector Register flags.
     */
    enum SpuriousIntVecFlags
    {
        APICEnable = 0x100
    };

    /**
     * Timer Divide Configuration Register flags.
     */
    enum DivideConfigFlags
    {
        Divide16 = 3
    };

    /**
     * Timer Register flags.
     */
    enum TimerFlags
    {
        PeriodicMode = (1 << 17)
    };

  public:

    /**
     * Constructor
     */
    IntelAPIC();

    /**
     * Get I/O object.
     *
     * @return I/O object reference.
     */
    IntelIO & getIO();

    /**
     * Get timer initial counter.
     *
     * @return Initial timer counter.
     */
    uint getCounter();

    /**
     * Initialize the APIC.
     *
     * @return Result code.
     */
    virtual Timer::Result initialize();

    /**
     * Busy wait a number of microseconds.
     *
     * @param microseconds The number of microseconds to wait at minimum.
     * @return Result code.
     */
    virtual Timer::Result wait(u32 microseconds);

    /**
     * Start the timer using PIT as reference timer.
     *
     * @param pit PIT instance used to measure the APIC bus speed for clock calibration.
     * @return Result code.
     */
    Timer::Result start(IntelPIT *pit);

    /**
     * Start the timer with initial counter.
     *
     * @param initialCounter The value of the InitialCount register.
     * @param hertz Hertz associated to the initial counter.
     * @return Result code.
     */
    Timer::Result start(uint initialCounter, uint hertz);

    /**
     * Enable hardware interrupt (IRQ).
     *
     * @param irq Interrupt Request number.
     * @return Result code.
     */
    virtual IntController::Result enable(uint irq);

    /**
     * Disable hardware interrupt (IRQ).
     *
     * @param irq Interrupt Request number.
     * @return Result code.
     */
    virtual IntController::Result disable(uint irq);

    /**
     * Clear hardware interrupt (IRQ).
     *
     * Clearing marks the end of an interrupt service routine
     * and causes the controller to trigger the interrupt again
     * on the next trigger moment.
     *
     * @param irq Interrupt Request number to clear.
     * @return Result code.
     */
    virtual IntController::Result clear(uint irq);

    /**
     * Send startup Intercore-Processor-Interrupt.
     *
     * @param cpuId CPU identifier to send startup IPI.
     * @param addr Start of execution address.
     * @return Result code.
     */
    IntController::Result sendStartupIPI(uint cpuId, Address addr);

  private:

    /** I/O object */
    IntelIO m_io;
};

#endif /* __LIBARCH_INTEL_APIC_H */
