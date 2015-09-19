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

#include <FreeNOS/API.h>
#include <CoreMessage.h>
#include "mpi.h"

int MPI_Init(int *argc, char ***argv)
{
    SystemInformation info;
    CoreMessage msg;

    // If we are master (node 0):
    msg.action = GetCoreCount;
    msg.from = SELF;
    msg.type = IPCType;
    IPCMessage(CORESRV_PID, API::SendReceive, &msg, sizeof(msg));

    // provide -n COUNT, --help and other stuff in here too.
    // to influence the launching of more MPI programs

    // Allocate memory space on the local processor for the whole
    // UniChannel array, NxN communication with MPI.
    // Then pass the channel offset physical address as an argument -addr 0x.... to spawn()

    // If we are slave (node N): 
    // read the -addr argument, and map the UniChannels into our address space.

/*
#error Process creation: send a message to the local CoreServer
#error which in turn sends a message to the destination core.
#error the message contains the physical address of the ELF executable and program arguments
#error later, a capability for the physical address will be applied to do access control.
 */

    return MPI_SUCCESS;
}

int MPI_Finalize(void)
{
    return MPI_SUCCESS;
}

int MPI_Get_processor_name(char *name, int *resultlen)
{
    return MPI_SUCCESS;
}

int MPI_Get_version(int *version, int *subversion)
{
    *version = 3;
    *subversion = 1;
    return MPI_SUCCESS;
}

int MPI_Get_library_version(char *version, int *resultlen)
{
    return MPI_SUCCESS;
}
