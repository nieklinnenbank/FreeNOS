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

#include <api/IPCMessage.h>
#include <api/ProcessCtl.h>
#include <arch/Process.h>
#include <MemoryServer.h>
#include <Config.h>
#include <stdio.h>
#include <sys/utsname.h>
#include <unistd.h>
#include "Shell.h"

Shell::Shell()
{
    printf("Shell(): PID %u\n", getpid());
}

int Shell::run()
{
    char *cmd;

    /* Read commands. */    
    while (true)
    {
	prompt();
	cmd = getCommand();

	if (strcmp(cmd, "ps") == 0)
	    ps();
	else if (strcmp(cmd, "uname") == 0)
	    doUname();
	else if (strcmp(cmd, "memstat") == 0)
	    memstat();
	else if (strcmp(cmd, "help") == 0)
	    help();
	else
	    printf("Command not found: '%s'\n", cmd);
    }
    return 0;
}

char * Shell::getCommand()
{
    static char line[1024];
    Size total = 0;

    /* Read a line. */
    while (total < sizeof(line))
    {
        /* Read a character. */
	getc(line + total);
	
	/* End of line reached? */
	if (line[total] != '\r' && line[total] != '\n')
	    printf("%c", line[total++]);
	else
	{
	    printf("\n"); break;
	}
    }
    line[total] = ZERO;
    return line;
}

void Shell::prompt()
{
    printf("# ");
}

void Shell::ps()
{
    ProcessInfo info;
    ProcessID   pid = 0;
    char *states[] = { "Running", "Ready", "Stopped", "Sleeping" };
    
    printf("PID STATE\n");
    
    while (!ProcessCtl(pid, Info, (Address) &info))
    {
	printf("%u %s\n", info.id, states[info.state % 4]);
	pid = info.id + 1;
    }
}

void Shell::doUname()
{
    struct utsname info;
    
    if (uname(&info) >= 0)
    {
	printf("%s %s %s %s %s\n",
		info.sysname,
		info.nodename,
		info.release,
		info.version,
		info.machine);
    }
}

void Shell::memstat()
{
    MemoryMessage msg;
    
    /* Query stats. */
    msg.action = MemoryUsage;
    
    /* Ask memory server for memory stats. */
    IPCMessage(MEMORY_PID, SendReceive, &msg);
    
    /* Print it. */
    printf("Total:     %u KB\n"
	   "Available: %u KB\n",
	   msg.bytes / 1024, msg.bytesFree / 1024);
}

void Shell::help()
{
    printf("ps      - Output list of Processes\n"
	   "uname   - Print UNIX name\n"
	   "memstat - Memory statistics\n"
	   "help    - This message\n");
}
