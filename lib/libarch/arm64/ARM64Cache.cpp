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

/*
 * Partly based on cache_v7.c from the Android project:
 *
 *  https://android.googlesource.com/device/ti/bootloader/uboot/+/c0eec2d5698a6722a195f4545064dccfb4010c16/arch/arm/cpu/armv7/cache_v7.c
 */

#include "ARM64Cache.h"

ARM64Cache::Result ARM64Cache::invalidate(ARM64Cache::Type type)
{
    switch (type)
    {
        case Instruction:
            return cleanInvalidate(type);

        case Data:
            return dataFlush(false);

        case Unified:
            invalidate(Instruction);
            invalidate(Data);
            break;
    }
    return Success;
}

ARM64Cache::Result ARM64Cache::cleanInvalidate(ARM64Cache::Type type)
{
    return Success;
}

ARM64Cache::Result ARM64Cache::cleanInvalidateAddress(Type type, Address addr)
{
    return Success;
}

ARM64Cache::Result ARM64Cache::cleanAddress(ARM64Cache::Type type, Address addr)
{
    return Success;
}

ARM64Cache::Result ARM64Cache::invalidateAddress(ARM64Cache::Type type, Address addr)
{
    return Success;
}

u32 ARM64Cache::getCacheLevelId() const
{
    return 0;
}

u32 ARM64Cache::getCacheLineSize() const
{
    return 0;
}

u32 ARM64Cache::readCacheSize(u32 level, u32 type) const
{
    return 0;
}

static inline s32 log_2_n_round_up(u32 n)
{
    s32 log2n = -1;
    u32 temp = n;

    while (temp) {
        log2n++;
        temp >>= 1;
    }

    if (n & (n - 1))
        return log2n + 1; // not power of 2 - round up
    else
        return log2n; // power of 2
}


ARM64Cache::Result ARM64Cache::flushLevel(u32 level, bool clean)
{
    return Success;
}

ARM64Cache::Result ARM64Cache::dataFlush(bool clean)
{
    return Success;
}

