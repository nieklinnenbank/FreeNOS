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

#include <Log.h>
#include "PrivExec.h"

Error PrivExecHandler(PrivOperation op, Address addr)
{
    ProcessManager *procs = Kernel::instance->getProcessManager();

    DEBUG("");

    switch (op)
    {
    case Idle:
        
        procs->setIdle(procs->current());
        irq_enable();

        while (true)
            idle();
    
    case Reboot:
        cpu_reboot();
        while (true) ;
        
    case Shutdown:
        cpu_shutdown();
        return API::Success;

    case WriteConsole:
        if (Log::instance)
        {
            (*Log::instance) << (char *)addr;
        }
        return API::Success;

    default:
        ;
    }
    return API::InvalidArgument;
}
