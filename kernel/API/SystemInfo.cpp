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

#include <FreeNOS/API.h>
#include <FreeNOS/Config.h>
#include <FreeNOS/Kernel.h>
#include <BitAllocator.h>

Error SystemInfoHandler(SystemInformation *info)
{
    BitAllocator *memory = Kernel::instance->getMemory();

    // TODO: Verify memory access

    // Fill in our current information
    info->version          = VERSIONCODE;
    info->memorySize       = memory->size();
    info->memoryAvail      = memory->available();
    info->memoryAllocator  = memory;

    // TODO: this interface could be improved using libarch?
    info->bootImageAddress = Kernel::instance->getBootImageAddress();
    info->bootImageSize    = Kernel::instance->getBootImageSize();

    // TODO: we dont have the commandline info of kernel yet.
    MemoryBlock::copy(info->cmdline, "", 64);
    return API::Success;
}
