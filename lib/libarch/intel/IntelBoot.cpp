/*
 * Copyright (C) 2009 Niek Linnenbank
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

#include <String.h>
#include <MemoryBlock.h>
#include <CoreInfo.h>
#include "IntelBoot.h"

extern Address __start, __end;

void multibootToCoreInfo(MultibootInfo *info)
{
    // Fill coreId and memory info
    MemoryBlock::set(&coreInfo, 0, sizeof(CoreInfo));
    coreInfo.coreId = 0;
    coreInfo.kernel.phys = (Address) &__start;
    coreInfo.kernel.size = ((Address) &__end - (Address) &__start);
    coreInfo.memory.phys = 0;

    // Limit maximum supported memory to 1GiB minus 128MiB
    // Unfortunately, libarch does not support RAM sizes of 1GiB and higher.
    // The reason is that the kernel maps only 1GiB minus 128MiB of RAM,
    // where the upper 128MiB is needed for the KernelPrivate section.
    // Mapping more than that will not work with SplitAllocator::toVirtual().
    // Therefore, use at maximum 1GiB minus 128MiB.
    if (info->memUpper <= (1024 * (1024 - 128))) {
        coreInfo.memory.size = (info->memUpper * 1024) + MegaByte(1);
    } else {
        coreInfo.memory.size = 1024 * 1024 * (1024 - 128);
    }

    // Fill the kernel command line
    MemoryBlock::copy(coreInfo.kernelCommand, (void *)info->cmdline, KERNEL_PATHLEN);

    // Fill the bootimage address
    for (Size n = 0; n < info->modsCount; n++)
    {
        MultibootModule *mod = (MultibootModule *) info->modsAddress;
        mod += n;
        String str((char *)(mod->string), false);

        // Is this the BootImage?
        if (str.match("*.img"))
        {
            coreInfo.bootImageAddress = mod->modStart;
            coreInfo.bootImageSize    = mod->modEnd - mod->modStart;
            break;
        }
    }
}
