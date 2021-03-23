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
#include <FreeNOS/ProcessManager.h>
#include <Log.h>
#include <SplitAllocator.h>
#include <CoreInfo.h>
#include <arm/ARMException.h>
#include <arm/ARMConstant.h>
#include <arm/broadcom/BroadcomInterrupt.h>
#include "ARMKernel.h"

ARMKernel::ARMKernel(CoreInfo *info)
    : Kernel(info)
    , m_exception(RAM_ADDR)
{
    ARMControl ctrl;

    NOTICE("");

    // Setup interrupt callbacks
    m_exception.install(ARMException::UndefinedInstruction, undefinedInstruction);
    m_exception.install(ARMException::SoftwareInterrupt, trap);
    m_exception.install(ARMException::PrefetchAbort, prefetchAbort);
    m_exception.install(ARMException::DataAbort, dataAbort);
    m_exception.install(ARMException::Reserved, reserved);
    m_exception.install(ARMException::IRQ, interrupt);
    m_exception.install(ARMException::FIQ, interrupt);

    // Set ARMCore modes
    ctrl.set(ARMControl::AlignmentFaults);

#ifdef ARMV6
    ctrl.unset(ARMControl::AlignmentCorrect);
    ctrl.unset(ARMControl::BigEndian);
#endif

    // First page is used for exception handlers
    m_alloc->allocate(info->memory.phys);

    // Allocate physical memory for the temporary stack.
    //
    // This is an area of 1MiB which must not be used. It is re-mapped on the
    // secondary cores to an identity-mapped area on the boot core and is currently
    // required in order to enable the early MMU and perform full kernel startup
    // until the first program starts.
    //
    // If this area is re-used after the kernel started, the SplitAllocator::toVirtual()
    // function will not translate properly, resulting in memory corruption.
    if (m_coreInfo->coreId == 0) {
        for (Size i = 0; i < (PAGESIZE*4); i += PAGESIZE)
            m_alloc->allocate(TMPSTACKADDR + i);
    } else {
        for (Size i = 0; i < MegaByte(1); i += PAGESIZE)
            m_alloc->allocate(info->memory.phys + TMPSTACKOFF + i);
    }
}

void ARMKernel::interrupt(CPUState state)
{
    ARMCore core;
    core.logException(&state);

    FATAL("core" << coreInfo.coreId << ": unhandled IRQ in procId = " <<
           Kernel::instance()->getProcessManager()->current()->getID());
}

void ARMKernel::undefinedInstruction(CPUState state)
{
    ARMCore core;
    core.logException(&state);

    FATAL("core" << coreInfo.coreId << ": procId = " <<
           Kernel::instance()->getProcessManager()->current()->getID());
}

void ARMKernel::prefetchAbort(CPUState state)
{
    ARMCore core;
    core.logException(&state);

    FATAL("core" << coreInfo.coreId << ": procId = " <<
           Kernel::instance()->getProcessManager()->current()->getID());
}

void ARMKernel::dataAbort(CPUState state)
{
    ARMCore core;
    core.logException(&state);

    FATAL("core" << coreInfo.coreId << ": procId = " <<
           Kernel::instance()->getProcessManager()->current()->getID());
}


void ARMKernel::reserved(CPUState state)
{
    ARMCore core;
    core.logException(&state);

    FATAL("core" << coreInfo.coreId << ": procId = " <<
           Kernel::instance()->getProcessManager()->current()->getID());
}

void ARMKernel::trap(volatile CPUState state)
{
    ProcessManager *mgr = Kernel::instance()->getProcessManager();
    ARMProcess *proc = (ARMProcess *) mgr->current(), *proc2;
    ProcessID procId = proc->getID();

    DEBUG("coreId = " << coreInfo.coreId << " procId = " << procId << " api = " << state.r0);

    // Execute the kernel call
    u32 r = Kernel::instance()->getAPI()->invoke(
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
            state.r0 = r;
            proc->setCpuState((const CPUState *)&state);
        }
        MemoryBlock::copy((void*)&state, proc2->cpuState(), sizeof(state));
    }
    else
        state.r0 = r;
}
