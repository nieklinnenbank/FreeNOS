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

#include <FreeNOS/System.h>
#include <Log.h>
#include <SplitAllocator.h>
#include <CoreInfo.h>
#include <arm/ARMInterrupt.h>
#include <arm/ARMConstant.h>
#include <arm/broadcom/BroadcomInterrupt.h>
#include "ARMKernel.h"

ARMKernel::ARMKernel(ARMInterrupt *intr,
                     CoreInfo *info)
    : Kernel(info)
#ifdef BCM2836
    , m_bcm(info->coreId)
#endif /* BMC2836 */
{
    ARMControl ctrl;

    NOTICE("");

    // Setup interrupt callbacks
    m_intControl = intr;
    intr->install(ARMInterrupt::UndefinedInstruction, undefinedInstruction);
    intr->install(ARMInterrupt::SoftwareInterrupt, trap);
    intr->install(ARMInterrupt::PrefetchAbort, prefetchAbort);
    intr->install(ARMInterrupt::DataAbort, dataAbort);
    intr->install(ARMInterrupt::Reserved, reserved);
    intr->install(ARMInterrupt::IRQ, interrupt);
    intr->install(ARMInterrupt::FIQ, interrupt);

    // Configure clocks and irqs. For BCM2836, only use the generic ARM timer
    // when running under Qemu. Unfortunately, Qemu dropped support for the
    // broadcom timer in recent versions. On hardware, use the broadcom timer.
#ifdef BCM2836
    u32 system_frequency = ctrl.read(ARMControl::SystemFrequency);
    NOTICE("sysfreq = " << system_frequency);
    if (system_frequency == 62500000)
    {
        // Use ARM generic timer
        m_timer = &m_armTimer;
        m_timerIrq = GTIMER_PHYS_1_IRQ;
        m_armTimer.setFrequency(100);

        // Setup IRQ routing
        m_bcm.setCoreTimerIrq(Broadcom2836::PhysicalTimer1, true);
    }
#endif /* BCM2836 */

    /* Default to broadcom timer and interrupt handling */
    if (m_timer == NULL)
    {
        m_timer = &m_bcmTimer;
        m_timerIrq = BCM_IRQ_SYSTIMERM1;
        m_bcmTimer.setFrequency( 250 ); /* trigger timer interrupts at 250Hz (clock runs at 1Mhz) */
        m_intControl->enable(BCM_IRQ_SYSTIMERM1);
    }

    // Set ARMCore modes
    ctrl.set(ARMControl::AlignmentFaults);

#ifdef ARMV6
    ctrl.unset(ARMControl::AlignmentCorrect);
    ctrl.unset(ARMControl::BigEndian);
#endif
}

void ARMKernel::interrupt(volatile CPUState state)
{
    ARMKernel *kernel = (ARMKernel *) Kernel::instance;
    ARMInterrupt *intr = (ARMInterrupt *) kernel->m_intControl;
    ARMProcess *proc = (ARMProcess *) Kernel::instance->getProcessManager()->current(), *next;
    bool tick;

    DEBUG("procId = " << proc->getID());

#ifdef BCM2836
    if (kernel->m_timer == &kernel->m_armTimer)
    {
        tick = kernel->m_bcm.getCoreTimerIrqStatus(Broadcom2836::PhysicalTimer1);
    }
    else
#endif /* BCM2836 */
    {
        tick = intr->isTriggered(BCM_IRQ_SYSTIMERM1);
    }

    if (tick)
    {
        kernel->m_timer->tick();
        next = (ARMProcess *)kernel->getProcessManager()->schedule();
        if (next)
        {
            proc->setCpuState((const CPUState *)&state);
            MemoryBlock::copy((void *)&state, next->cpuState(), sizeof(state));
        }
    }

    for (uint i = kernel->m_timerIrq + 1; i < 64; i++)
    {
        if (intr->isTriggered(i))
        {
            kernel->executeIntVector(i, (CPUState *)&state);
        }
    }

}

void ARMKernel::undefinedInstruction(CPUState state)
{
    ARMCore core;
    core.logException(&state);
    FATAL("procId = " << Kernel::instance->getProcessManager()->current()->getID());
    for(;;);
}

void ARMKernel::prefetchAbort(CPUState state)
{
    ARMCore core;
    core.logException(&state);
    FATAL("procId = " << Kernel::instance->getProcessManager()->current()->getID());
    for(;;);
}

void ARMKernel::dataAbort(CPUState state)
{
    ARMCore core;
    core.logException(&state);
    FATAL("procId = " << Kernel::instance->getProcessManager()->current()->getID());
    for(;;);
}


void ARMKernel::reserved(CPUState state)
{
    ARMCore core;
    core.logException(&state);
    FATAL("procId = " << Kernel::instance->getProcessManager()->current()->getID());
    for(;;);
}

void ARMKernel::trap(volatile CPUState state)
{
    ProcessManager *mgr = Kernel::instance->getProcessManager();
    ARMProcess *proc = (ARMProcess *) mgr->current(), *proc2;
    ProcessID procId = proc->getID();

    DEBUG("procId = " << procId << " api = " << state.r0);

    // Execute the kernel call
    u32 r = Kernel::instance->getAPI()->invoke(
        (API::Number) state.r0,
                      state.r1,
                      state.r2,
                      state.r3,
                      state.r4,
                      state.r5
    );

    // Did we change process?
    proc2 = (ARMProcess *) mgr->current();
    DEBUG("result = " << r << " scheduled = " << (bool)(proc != proc2));

    if (proc != proc2)
    {
        // Only if the previous process still exists (not killed in API)
        if (mgr->get(procId) != NULL)
        {
            state.r0 = 0;
            proc->setCpuState((const CPUState *)&state);
        }
        MemoryBlock::copy((void*)&state, proc2->cpuState(), sizeof(state));
    }
    else
        state.r0 = r;
}
