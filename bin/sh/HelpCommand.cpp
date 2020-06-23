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

#include <stdio.h>
#include <HashIterator.h>
#include "HelpCommand.h"
#include "ShellCommand.h"
#include "Shell.h"

HelpCommand::HelpCommand(Shell *shell)
    : ShellCommand("help", 0)
{
    m_shell = shell;
    m_help  = "Print the help message";
}

int HelpCommand::execute(const Size nparams, const char **params)
{
    printf("List of builtin Shell commands:\r\n\r\n");

    for (HashIterator<String, ShellCommand *> i(m_shell->getCommands()); i.hasCurrent(); i++)
    {
        printf("%s -- %s\r\n", i.current()->getName(), i.current()->getHelp());
    }
    printf("\r\n");
    printf("Type 'ls /bin' to see available user commands\r\n");

    return 0;
}
