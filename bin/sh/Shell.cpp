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
#include <arch/Process.h>
#include <arch/CPU.h>
#include <MemoryServer.h>
#include <FileSystemMessage.h>
#include <VirtualFileSystem.h>
#include <Config.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <fcntl.h>
#include "Shell.h"

Shell::Shell()
{
    printf("Shell(): PID %u\n", getpid());
}

int Shell::run()
{
    char *cmdStr, *argv[MAX_ARGV];
    ShellCommand *cmd;
    Size argc;

    /* Read commands. */    
    while (true)
    {
	/* Print the prompt. */
	prompt();
	
	/* Wait for a command string. */
	cmdStr = getCommand();
	
	/* Attempt to extract arguments. */
	argc = parse(cmdStr, argv, MAX_ARGV);
	
	/* Do we have a matching ShellCommand? */
	if (!(cmd = ShellCommand::byName(argv[0])))
	{
	    printf("Command not found: '%s'\n", cmdStr);
	}
	/* Enough arguments given? */
	else if (argc < cmd->getMinimumParams())
	{
	    printf("%s: not enough arguments (%u required)\n",
		    cmd->getName(), cmd->getMinimumParams());
	}
	/* Execute it. */
	else
	{
	    cmd->execute(argc - 1, argv + 1);
	}
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

Size Shell::parse(char *cmdline, char **argv, Size maxArgv)
{
    Size argc;
    
    for (argc = 0; argc < maxArgv && *cmdline; argc++)
    {
	while (*cmdline && *cmdline == ' ')
	    cmdline++;
	
	argv[argc] = cmdline;
	
	while (*cmdline && *cmdline != ' ')
	    cmdline++;
	
	if (*cmdline) *cmdline++ = ZERO;
    }
    return argc;
}
