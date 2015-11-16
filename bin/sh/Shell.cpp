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

#include <TerminalCodes.h>
#include "Shell.h"
#include "ChangeDirCommand.h"
#include "ExitCommand.h"
#include "StdioCommand.h"
#include "WriteCommand.h"
#include "HelpCommand.h"
#include "TimeCommand.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

Shell::Shell()
{
    registerCommand(new ChangeDirCommand());
    registerCommand(new ExitCommand());
    registerCommand(new StdioCommand());
    registerCommand(new WriteCommand());
    registerCommand(new HelpCommand(this));
    registerCommand(new TimeCommand());
}

int Shell::run()
{
    char *cmdStr;

    /* Read commands. */    
    while (true)
    {
	/* Print the prompt. */
	prompt();
	
	/* Wait for a command string. */
	cmdStr = getInput();
	
	/* Enough input? */
	if (strlen(cmdStr) == 0)
	{
	    continue;
	}
	/* Execute the command. */
	executeInput(cmdStr);
    }
    return EXIT_SUCCESS;
}

int Shell::executeInput(char *command)
{
    char *argv[MAX_ARGV];
    char tmp[128];
    ShellCommand *cmd;
    Size argc;
    int pid, status;
    bool background;

    /* Valid argument? */
    if (!strlen(command))
    {
        return EXIT_SUCCESS;
    }
    /* Attempt to extract arguments. */
    argc = parse(command, argv, MAX_ARGV, &background);

    /* Ignore comments */
    if (argv[0][0] == '#')
        return EXIT_SUCCESS;

    /* Do we have a matching ShellCommand? */
    if (!(cmd = getCommand(argv[0])))
    {
        /* If not, try to execute it as a file directly. */
        if ((pid = forkexec(argv[0], (const char **) argv)) != -1)
        {
            if (!background)
            {
                waitpid(pid, &status, 0);
                return status;
            } else
                return EXIT_SUCCESS;
        }
        /* Try to find it on the livecd filesystem. (temporary hardcoded PATH) */
        else if (snprintf(tmp, sizeof(tmp), "/bin/%s", argv[0]) &&
                (pid = forkexec(tmp, (const char **) argv)) != -1)
        {
            if (!background)
            {
                waitpid(pid, &status, 0);
                return status;
            } else
                return EXIT_SUCCESS;
        }
        else
            printf("forkexec '%s' failed: %s\r\n", argv[0],
                    strerror(errno));
    }
    /* Enough arguments given? */
    else if (argc - 1 < cmd->getMinimumParams())
    {
        printf("%s: not enough arguments (%u required)\r\n",
                cmd->getName(), cmd->getMinimumParams());
    }
    /* Execute it. */
    else
    {
        return cmd->execute(argc - 1, argv + 1);
    }
    /* Not successful. */
    return EXIT_FAILURE;
}

char * Shell::getInput()
{
    static char line[1024];
    Size total = 0;

    /* Read a line. */
    while (total < sizeof(line))
    {
        /* Read a character. */
	read(0, line + total, 1);
	
	/* Process character. */
	switch (line[total])
	{
	    case '\r':
	    case '\n':
	    	printf("\r\n");
		line[total] = ZERO;
		return line;

	    case '\b':
		if (total > 0)
		{
		    total--;
		    printf("\b \b");
		}
		break;
	    
	    default:
		printf("%c", line[total]);
		total++;
		break;
	}
    }
    line[total] = ZERO;
    return line;
}

void Shell::prompt()
{
    char host[128], cwd[128];
    
    /* Retrieve current hostname. */
    gethostname(host, sizeof(host));
    
    /* Retrieve current working directory. */
    getcwd(cwd, sizeof(cwd));
    
    /* Print out the prompt. */
    printf(WHITE "(" GREEN "%s" WHITE ") " BLUE "%s" WHITE " # ",
	   host, cwd);
}

HashTable<String, ShellCommand *> & Shell::getCommands()
{
    return m_commands;
}

ShellCommand * Shell::getCommand(const char *name)
{
    return m_commands.get(name) ? *m_commands.get(name) : ZERO;
}

void Shell::registerCommand(ShellCommand *command)
{
    m_commands.insert(command->getName(), command);
}

Size Shell::parse(char *cmdline, char **argv, Size maxArgv, bool *background)
{
    Size argc;

    *background = false;
    
    for (argc = 0; argc < maxArgv && *cmdline; argc++)
    {
        while (*cmdline && *cmdline == ' ')
            cmdline++;

        if (*cmdline == '&')
        {
            *background = true;
            break;
        }

        argv[argc] = cmdline;

        while (*cmdline && *cmdline != ' ')
            cmdline++;

        if (*cmdline) *cmdline++ = ZERO;
    }
    argv[argc] = ZERO;
    return argc;
}
