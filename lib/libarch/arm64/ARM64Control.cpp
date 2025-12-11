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

#include "ARM64Control.h"
#include <Log.h>

namespace ARM64Control {

u64 read(Register reg)
{
    u64 r = 0;
    switch (reg)
    {
        case SystemControl: {
            asm volatile ("mrs %0, sctlr_el1" : "=r" (r));
            break;
        }
        case SystemFrequency: {
            asm volatile ("mrs %0, cntfrq_el0" : "=r" (r));
            break;
        }
        case MemoryModelFeature: {
            asm volatile ("mrs %0, id_aa64mmfr0_el1" : "=r" (r));
            break;
        } 
        case PhysicalTimerCount: {
            asm volatile ("mrs %0, cntpct_el0" : "=r" (r));
            break;
        }
        case PhysicalTimerValue: {
            asm volatile ("mrs %0, cntp_tval_el0" : "=r" (r));
            break;
        }
        case PhysicalTimerControl: {
            asm volatile ("mrs %0, cntp_ctl_el0" : "=r" (r));
            break;
        }
        case DAIF: {
            asm volatile ("mrs %0, DAIF" : "=r" (r));
            break;
        }
        default: break;
    }
    return r;
}

void write(Register reg, u64 value)
{
    switch (reg)
    {
        case SystemControl:  {
            asm volatile ("msr sctlr_el1, %0" : : "r" (value));
            break;
        }
        case TranslationTable0:  {
            asm volatile ("msr ttbr0_el1, %0" : : "r" (value));
            break;
        }
        case TranslationTable1:  {
            asm volatile ("msr ttbr1_el1, %0" : : "r" (value));
            break;
        }
        case TranslationTableCtrl:  {
            asm volatile ("msr tcr_el1, %0; isb" : : "r" (value));
            break;
        }
        case VectorBaseAddress: {
            asm volatile ("msr vbar_el1, %0" : : "r" (value));
            break;
        }
        case MemoryAttrIndirection: {
            asm volatile ("msr mair_el1, %0" : : "r" (value));
            break;
        }
        case PhysicalTimerValue: {
            asm volatile ("msr cntp_tval_el0, %0" : : "r" (value));
            break;
        }
        case PhysicalTimerControl: {
            asm volatile ("msr cntp_ctl_el0, %0" : : "r" (value));
            break;
        }
        default: break;
    }
}

void set(Register reg, u64 flags)
{
    u64 val = read(reg);
    val |= flags;
    write(reg, val);
}

void unset(Register reg, u64 flags)
{
    u64 val = read(reg);
    val &= ~(flags);
    write(reg, val);
}

void set(AuxControlFlags flags)
{
    set(AuxControl, flags);
}

void set(DomainControlFlags flags)
{
    set(DomainControl, flags);
}

}
