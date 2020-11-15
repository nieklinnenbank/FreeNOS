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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <Log.h>
#include <SystemClock.h>
#include "Shell.h"
#include "TimeCommand.h"

TimeCommand::TimeCommand(Shell *shell)
    : ShellCommand("time", 1)
    , m_shell(shell)
{
    m_help = "Measure the execution time of a program";
}

int TimeCommand::execute(const Size nparams, const char **params)
{
    SystemClock t1, t2;

    // Get timestamp before
    t1.now();

    // Run command
    int result = m_shell->executeInput(nparams, params, false);
    if (result != EXIT_SUCCESS)
    {
        return result;
    }

    // Get timestamp after
    t2.now();

    // Print time measured
    printf("\r\nTime: ");
    t1.printDiff(t2);
    return result;
}
