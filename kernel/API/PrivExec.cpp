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

#include <API/PrivExec.h>
#include <Arch/Interrupt.h>
#include <FreeNOS/Scheduler.h>
#include <Arch/CPU.h>
#include <FreeNOS/Kernel.h>
#include <Error.h>

int PrivExecHandler(PrivOperation op)
{
    switch (op)
    {
	case Idle:
	    
	    scheduler->setIdle(scheduler->current());
	    irq_enable();
	    
	    while (true)
		idle();
	
	case Reboot:
	    reboot();
	    while (true) ;
	    
	case Shutdown:
	    shutdown();
	    return ESUCCESS;

	default:
	    ;
    }
    return EINVAL;
}

INITAPI(PRIVEXEC, PrivExecHandler)
