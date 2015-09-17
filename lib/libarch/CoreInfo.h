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

#ifndef __LIBARCH_COREINFO_H
#define __LIBARCH_COREINFO_H

#define KERNEL_PATHLEN 64

// TODO: needed by IntelBoot16.S.
// TODO: warning depends on sizeof(Memory::Access) which is an emum
#define COREINFO_SIZE  (KERNEL_PATHLEN + (8 * 4) + (4 * 4) + (4 * 4))

#ifndef __ASSEMBLER__

#include <Macros.h>
#include <Types.h>
#include <Memory.h>

typedef struct CoreInfo
{
    uint booted;
    uint coreId;
    Memory::Range memory;

    /** Kernel entry point */
    Address kernelEntry;

    /** Kernel memory range */
    Memory::Range kernel;

    /** Kernel command */
    char kernelCommand[KERNEL_PATHLEN];

    Address bootImageAddress;
    Address bootImageSize;
    Address coreChannelAddress;
    Size coreChannelSize;

    /** Arch-specific timer counter */
    uint timerCounter;

    // TODO: fix this in libstd. we should not need to do this for Index.
    bool operator == (const struct CoreInfo & info) const
    {
        return false;
    }
    bool operator != (const struct CoreInfo & info) const
    {
        return true;
    }
}
CoreInfo;

/**
 * Local CoreInfo instance.
 *
 * Architecture is responsible for filling the
 * CoreInfo struct instance for the core on which it
 * executes.
 */
extern CoreInfo coreInfo;

#endif /* __ASSEMBLER__ */
#endif /* __LIBARCH_COREINFO_H */
