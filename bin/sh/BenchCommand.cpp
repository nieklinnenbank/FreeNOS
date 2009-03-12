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

#include <arch/CPU.h>
#include <api/ProcessCtl.h>
#include <api/IPCMessage.h>
#include <MemoryMessage.h>
#include <Config.h>
#include <unistd.h>
#include "BenchCommand.h"
#include "ShellCommand.h"

int BenchCommand::execute(Size nparams, char **params)
{
    u64 t1 = 0, t2 = 0;
    ProcessID pid = 0;
    ShellCommand *cmd;
    ProcessInfo info;
    
    if (nparams == 0)
    {
	t1 = timestamp();
        pid = ProcessCtl(SELF, GetPID);
        t2 = timestamp();
	
        printf("SystemCall (GetPID) Ticks: %u\n", t2 - t1);
	
	t1 = timestamp();
	ProcessCtl(SELF, InfoPID, (Address) &info);
	t2 = timestamp();

	printf("SystemCall (InfoPID) Ticks: %u\n", t2 - t1);

	t1 = timestamp();
	ProcessCtl(SELF, Schedule);
	t2 = timestamp();

	printf("SystemCall (Schedule) Ticks: %u\n", t2 - t1);

	t1 = timestamp();
	getpid();
	t2 = timestamp();

	printf("IPC Ticks: %u\n", t2 - t1);
    }
    else
    {
	if ((cmd = ShellCommand::byName(params[0])))
	{
	    if (nparams - 1 >= cmd->getMinimumParams())
	    {
		t1 = timestamp();
		cmd->execute(nparams - 1, params + 1);
		t2 = timestamp();
		printf("%s Ticks: %u\n", cmd->getName(), t2 - t1);
	    }
	}
    }
    return 0;
}

INITOBJ(BenchCommand, benchCmd, DEFAULT)
