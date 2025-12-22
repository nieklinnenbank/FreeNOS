/*
 * Copyright (C) 2025 Ivan Tan
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
#include <SplitAllocator.h>
#include <CoreInfo.h>
#include <Log.h>
#include <arm64/ARM64Exception.h>
#include <arm64/ARM64Constant.h>
#include <arm64/ARM64Control.h>
#include "ARM64Kernel.h"

extern Address __start;

void ARM64Kernel::FatalHandler(volatile CPUState state)
{
    FATAL("This is a test");
}

void ARM64Kernel::SyncExceptionEL0(volatile CPUState state)
{
    u64 ec = exception_code(state.esr);

    switch(ec) {
        case 0x25: //Data Abort
            ERROR("Failed to access " << (void *)state.far);
            break;
        case 0x15:
            trap(state);
        default:
            //omitted
            break;
    }
}

void ARM64Kernel::SyncExceptionEL1(volatile CPUState state)
{
    u64 ec = exception_code(state.esr);
    NOTICE("Unexpected m_exception in EL1 called from EL1 ec="<<(void *)ec);
    NOTICE("ESR_EL1 = "<<(void *)state.esr);

    switch(ec) {
        case 0x25: //Data Abort
            ERROR("Failed to access " << (void *)state.far);
            break;
        case 0x15:
            ERROR("SVC instruction execution in AArch64 state.");
        default:
            //omitted
            break;
    }
    while(1) {};
}

ARM64Kernel::ARM64Kernel(CoreInfo *info)
    : Kernel(info)
    , m_exception()
{
    NOTICE("");

    // Setup interrupt callbacks
    m_exception.install(ARM64Exception::Sync_SP_EL0, FatalHandler);
    m_exception.install(ARM64Exception::IRQ_SP_EL0, FatalHandler);
    m_exception.install(ARM64Exception::FIQ_SP_EL0, FatalHandler);
    m_exception.install(ARM64Exception::SError_SP_EL0, FatalHandler);
    m_exception.install(ARM64Exception::Sync_SP_ELx, SyncExceptionEL1);
    m_exception.install(ARM64Exception::IRQ_SP_ELx, FatalHandler);
    m_exception.install(ARM64Exception::FIQ_SP_ELx, FatalHandler);
    m_exception.install(ARM64Exception::SError_SP_ELx, FatalHandler);
    m_exception.install(ARM64Exception::Sync_Lower_EL, SyncExceptionEL0);
    m_exception.install(ARM64Exception::IRQ_Lower_EL, FatalHandler);
    m_exception.install(ARM64Exception::FIQ_Lower_EL, FatalHandler);
    m_exception.install(ARM64Exception::SError_Lower_EL, FatalHandler);
    m_exception.install(ARM64Exception::Sync_Lower_EL_AA32, FatalHandler);
    m_exception.install(ARM64Exception::IRQ_Lower_EL_AA32, FatalHandler);
    m_exception.install(ARM64Exception::FIQ_Lower_EL_AA32, FatalHandler);
    m_exception.install(ARM64Exception::SError_Lower_EL_AA32, FatalHandler);

    // First page is reserved
    for (Size i = 0; i < info->kernel.phys; i += PAGESIZE)
        m_alloc->allocate(i);

    // Allocate physical memory for the temporary stack.
    if (m_coreInfo->coreId == 0) {
        for (Size i = 0; i < (PAGESIZE*16); i += PAGESIZE)
            m_alloc->allocate(TMPSTACKADDR + i);
    }
}

void ARM64Kernel::trap(volatile CPUState &state)
{
    ProcessManager *mgr = Kernel::instance()->getProcessManager();
    ARM64Process *proc = (ARM64Process *) mgr->current(), *proc2;
    ProcessID procId = proc->getID();

    DEBUG("coreId = " << coreInfo.coreId << " procId = " << procId << " api = " << (Address)state.x8);
    DEBUG("args = " << (void *)state.x0 << ", " << (void *)state.x1 << ", " << (void *)state.x2
            << ", " << (void *)state.x3 << "," << (void *)state.x4);

    // Execute the kernel call
    u32 r = Kernel::instance()->getAPI()->invoke(
        (API::Number) state.x8,
                      state.x0,
                      state.x1,
                      state.x2,
                      state.x3,
                      state.x4
    );

    // Did we change process?
    proc2 = (ARM64Process *) mgr->current();
    DEBUG("result = " << r << " scheduled = " << (bool)(proc != proc2));

    if (proc != proc2)
    {
        // Only if the previous process still exists (not killed in API)
        if (mgr->get(procId) != NULL)
        {
            state.x0 = r;
            proc->setCpuState((const CPUState *)&state);
        }
        MemoryBlock::copy((void*)&state, proc2->cpuState(), sizeof(state));
    }
    else
        state.x0 = r;
}
