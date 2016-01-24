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
     * Get shell command.
     *
     * @param name ShellCommand name.
     * @return ShellCommand object pointer.
     */
    ShellCommand * getCommand(const char *name);

    /**
     * Get all shell commands.
     *
     * @return HashTable with ShellCommands
     */
    HashTable<String, ShellCommand *> & getCommands();

    /**
     * Register a new ShellCommand.
     *
     * @param command ShellCommand object pointer.
     */
    void registerCommand(ShellCommand *command);

    /**
     * Executes the Shell by entering an infinite loop.
     * @return Never.
     */
    int run();

    /**
     * Executes the given input.
     *
     * @param cmdline Input to execute.
     * @return Exit status of the command.
     */
    int executeInput(char *cmdline);

  private:
    
    /**
     * Fetch a command text from standard input.
     * @return Pointer to a command text.
     */
    char * getInput();


    /**
     * Output a prompt.
     */
    void prompt();

    /**
     * Parses an input string into separate pieces.
     * @param cmdline Command input string.
     * @param argv Argument list buffer.
     * @param maxArgv Maximum number of entries in argv.
     * @param background True if process must run in the background.
     * @return Number of parsed arguments.
     */
    Size parse(char *cmdline, char **argv, Size maxArgv, bool *background);

    /** All known ShellCommands. */
    HashTable<String, ShellCommand *> m_commands;
};

#endif /* __SH_SHELL */
