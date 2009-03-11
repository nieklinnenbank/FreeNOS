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

#ifndef __SH_SHELLCOMMAND
#define __SH_SHELLCOMMAND

#include <String.h>
#include <HashTable.h>
#include <Types.h>

/**
 * Builtin command for the Shell.
 * @see Shell
 */
class ShellCommand
{
    public:

	/**
	 * Constructor function.
	 * @param name Unique name of the command.
	 * @param minParams Minimum number of parameters required by this command.
	 */
	ShellCommand(const char *name, Size minParams = 0);

	/**
	 * Destructor function.
	 */
	virtual ~ShellCommand();

	/**
	 * Fetch the name of this command.
	 * @return Character array pointer.
	 */
	const char * getName()
	{
	    return name;
	}

	/**
	 * Get the minimum number of parameters required.
	 * @return Minimum number of parameters required.
	 */
	Size getMinimumParams()
	{
	    return minParams;
	}

	/**
	 * Get the help string for this command.
	 * @return Pointer to character string describing what the command does.
	 */
	virtual const char * help() = 0;

	/**
	 * Executes the command.
	 * @param nparams Number of parameters given.
	 * @param params Array of parameters.
	 * @return Error code or zero on success.
	 */
	virtual int execute(Size nparams, char **params) = 0;

	/**
	 * Lookup a command via it's name.
	 * @param name Unique name of the command to find.
	 * @return Pointer to a ShellCommand object if found, ZERO otherwise.
	 */
	static ShellCommand * byName(char *name);

    protected:

	/** Unique name of the command. */
	const char *name;
    
	/** Minimum number of parameters required. */
	Size minParams;
    
	/** All known ShellCommands. */
	static HashTable<String, ShellCommand> commands;
};

#endif /* __SH_SHELLCOMMAND */
