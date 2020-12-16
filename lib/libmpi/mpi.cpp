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

#include <Assert.h>
#include "MpiBackend.h"
#include "mpi.h"

static MpiBackend *mpiBackend = MpiBackend::create();

extern C int MPI_Init(int *argc, char ***argv)
{
    assert(mpiBackend != ZERO);
    return mpiBackend->initialize(argc, argv);
}

extern C int MPI_Finalize(void)
{
    assert(mpiBackend != ZERO);
    return mpiBackend->terminate();
}

extern C int MPI_Send(const void *buf,
                      int count,
                      MPI_Datatype datatype,
                      int dest,
                      int tag,
                      MPI_Comm comm)
{
    assert(mpiBackend != ZERO);
    return mpiBackend->send(buf, count, datatype, dest, tag, comm);
}

extern C int MPI_Recv(void *buf,
                      int count,
                      MPI_Datatype datatype,
                      int source,
                      int tag,
                      MPI_Comm comm,
                      MPI_Status *status)
{
    assert(mpiBackend != ZERO);
    return mpiBackend->receive(buf, count, datatype, source, tag, comm, status);
}

extern C int MPI_Comm_rank(MPI_Comm comm,
                           int *rank)
{
    assert(mpiBackend != ZERO);
    return mpiBackend->getCommRank(comm, rank);
}

extern C int MPI_Comm_size(MPI_Comm comm,
                           int *size)
{
    assert(mpiBackend != ZERO);
    return mpiBackend->getCommSize(comm, size);
}
