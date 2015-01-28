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

#ifndef __SH_STDIOCOMMAND
#define __SH_STDIOCOMMAND

#include <Factory.h>
#include <Types.h>
#include "ShellCommand.h"

/**
 * Change the standard Input/Output of the shell.
 */
class StdioCommand : public ShellCommand, public Factory<StdioCommand>
{
    public:

    /**
     * Constructor function.
     */
    StdioCommand();

    /**
     * Get the help string for this command.
     * @return Pointer to character string describing what the command does.
     */
    const char * help();

    /**
     * Executes the command.
     * @param nparams Number of parameters given.
     * @param params Array of parameters.
     * @return Error code or zero on success.
     */
    int execute(Size nparams, char **params);
};

#endif /* __SH_STDIOCOMMAND */
