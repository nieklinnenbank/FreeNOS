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

#include "ARM64Control.h"

namespace ARM64Control {

u64 read(Register reg)
{
    u64 r = 0;
    switch (reg)
    {
#if 0
        case MainID:                  return mrc(p15, 0, 0, c0,  c0);
        case CoreID:                  return mrc(p15, 0, 5, c0,  c0);
#endif
        case SystemControl: {
            asm volatile ("mrs %0, sctlr_el1" : "=r" (r));
            break;
        }
#if 0
        case AuxControl:              return mrc(p15, 0, 1, c1,  c0);
#endif
#if 0
        case TranslationTable0:       return mrc(p15, 0, 0, c2,  c0);
        case TranslationTable1:       return mrc(p15, 0, 1, c2,  c0);
        case TranslationTableCtrl:    return mrc(p15, 0, 2, c2,  c0);
#endif
#if 0
        case DomainControl:           return mrc(p15, 0, 0, c3,  c0);
        case UserProcID:              return mrc(p15, 0, 4, c13, c0);
        case InstructionFaultAddress: return mrc(p15, 0, 2, c6, c0);
        case InstructionFaultStatus:  return mrc(p15, 0, 1, c5, c0);
        case DataFaultAddress:        return mrc(p15, 0, 0, c6, c0);
        case DataFaultStatus:         return mrc(p15, 0, 0, c5, c0);
        case SystemFrequency:         return mrc(p15, 0, 0, c14, c0);
#endif
        case MemoryModelFeature: {
            asm volatile ("mrs %0, id_aa64mmfr0_el1" : "=r" (r));
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
#if 0
        case AuxControl:            mcr(p15, 0, 1, c1,  c0, value); break;
#endif
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
#if 0
        case DomainControl:         mcr(p15, 0, 0, c3,  c0, value); break;
        case CacheClear:            mcr(p15, 0, 0, c7,  c7, value); break;
        case DataCacheClean:        mcr(p15, 0, 0, c7, c14, value); break;
        case FlushPrefetchBuffer:   flushPrefetchBuffer(); break;
        case InstructionCacheClear: mcr(p15, 0, 0, c7,  c5, value); break;
        case InstructionTLBClear:   mcr(p15, 0, 0, c8,  c5, value); break;
        case DataTLBClear:          mcr(p15, 0, 0, c8,  c6, value); break;
        case UnifiedTLBClear:       mcr(p15, 0, 0, c8,  c7, value); break;
        case UserProcID:            mcr(p15, 0, 4, c13, c0, value); break;
#endif
        case MemoryAttrIndirection: {
            asm volatile ("msr mair_el1, %0" : : "r" (value));
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
