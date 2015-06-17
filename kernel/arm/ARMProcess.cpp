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

#include <Log.h>
#include "ARMProcess.h"

ARMProcess::ARMProcess(ProcessID id, Address entry, bool privileged)
    : Process(id, entry, privileged)
{
    NOTICE("id =" << id << " entry =" << entry << " privileged = " << privileged);
}

ARMProcess::~ARMProcess()
{
    NOTICE("");
}

void ARMProcess::execute(Process *previous)
{
    DEBUG("");
}
