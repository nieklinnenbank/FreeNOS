/*
 * Copyright (C) 2020 Niek Linnenbank
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

#include <FreeNOS/User.h>
#include <stdio.h>
#include "Shutdown.h"

Shutdown::Shutdown(int argc, char **argv)
    : POSIXApplication(argc, argv)
{
    parser().setDescription("Power off or restart the system");
    parser().registerFlag('p', "poweroff", "Power off the system (the default)");
    parser().registerFlag('r', "reboot", "Restart the system");
}

Shutdown::~Shutdown()
{
}

Shutdown::Result Shutdown::exec()
{
    // If no arguments given, power off the system. */
    if (arguments().getFlags().count() == 0)
    {
        printf("Power off\r\n");
        PrivExec(ShutdownSystem);
    }
    // Print everything?
    else if (arguments().get("reboot"))
    {
        printf("Reboot\r\n");
        PrivExec(RebootSystem);
    }

    return Success;
}
