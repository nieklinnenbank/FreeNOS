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

#ifndef __SH_SHELL
#define __SH_SHELL

#include <Types.h>
#include "ShellCommand.h"

/** Maximum number of supported command arguments. */
#define MAX_ARGV 16

/**
 * Very basic command shell.
 */
class Shell
{
    public:

        /**
         * Constructor
         */
        Shell();

	/**
	 * Executes the Shell by entering an infinite loop.
	 * @return Never.
	 */
	int run();

	/**
	 * Executes the given command.
	 * @param cmdline Command to execute.
	 * @return Exit status of the command.
	 */
	int execute(char *cmdline);

    private:
    
	/**
	 * Fetch a command from standard input.
	 * @return Pointer to a command.
	 */
	char * getCommand();
    
	/**
	 * Output a prompt.
	 */
	void prompt();

	/**
	 * Parses an input string into separate pieces.
	 * @param cmdline Command input string.
	 * @param argv Argument list buffer.
	 * @param maxArgv Maximum number of entries in argv.
	 * @return Number of parsed arguments.
	 */	
	Size parse(char *cmdline, char **argv, Size maxArgv);
};

#endif /* __SH_SHELL */
