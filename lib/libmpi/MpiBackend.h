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

#ifndef __LIB_LIBMPI_MPIBACKEND_H
#define __LIB_LIBMPI_MPIBACKEND_H

#include <Types.h>
#include <Factory.h>
#include "mpi.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libmpi
 * @{
 */

/**
 * Represents a Message Passing Interface (MPI) implementation backend.
 */
class MpiBackend : public AbstractFactory<MpiBackend>
{
  public:

    /**
     * Result code
     *
     * The same MPI_* values of mpi.h are used for Result values
     *
     * @see mpi.h
     */
    typedef int Result;

  public:

    /**
     * Initialize the backend
     *
     * @param argc Argument count pointer
     * @param argv Argument values array pointer
     *
     * @return Result code
     */
    virtual Result initialize(int *argc,
                              char ***argv) = 0;

    /**
     * Terminate the backend
     *
     * @return Result code
     */
    virtual Result terminate() = 0;

    /**
     * Retrieve communication rank (core id)
     *
     * @param comm Communication reference
     * @param rank Output the communication rank number
     *
     * @return Result code
     */
    virtual Result getCommRank(MPI_Comm comm,
                               int *rank) = 0;

    /**
     * Retrieve communication size (total cores)
     *
     * @param comm Communication reference
     * @param size Output the communication size
     *
     * @return Result code
     */
    virtual Result getCommSize(MPI_Comm comm,
                               int *size) = 0;

    /**
     * Synchronous send data
     *
     * @param buf Input data buffer
     * @param count Number of data items
     * @param datatype Type of data
     * @param dest Destination to send to (core id)
     * @param tag Optional data identifier to send
     * @param comm Communication reference
     *
     * @return Result code
     */
    virtual Result send(const void *buf,
                        int count,
                        MPI_Datatype datatype,
                        int dest,
                        int tag,
                        MPI_Comm comm) = 0;

    /**
     * Synchronous receive data
     *
     * @param buf Output data buffer
     * @param count Number of data items
     * @param datatype Type of data
     * @param source Source to receive data from (core id)
     * @param tag Optional data identifier to receive
     * @param comm Communication reference
     * @param status Output the MPI status
     *
     * @return Result code
     */
    virtual Result receive(void *buf,
                           int count,
                           MPI_Datatype datatype,
                           int source,
                           int tag,
                           MPI_Comm comm,
                           MPI_Status *status) = 0;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBMPI_MPIBACKEND_H */
