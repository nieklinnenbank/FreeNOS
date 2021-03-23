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

#include <Log.h>
#include <mpi.h>
#include "MpiPing.h"

MpiPing::MpiPing(int argc, char **argv)
    : POSIXApplication(argc, argv)
    , m_mpiInitResult(MPI_Init(&m_argc, &m_argv))
    , m_id(0)
{
    parser().setDescription("Send ping message to cores via MPI");
}

MpiPing::~MpiPing()
{
    DEBUG("");
}

MpiPing::Result MpiPing::initialize()
{
    if (m_mpiInitResult != MPI_SUCCESS)
    {
        ERROR("failed to initialize MPI: result = " << m_mpiInitResult);
        return IOError;
    }

    int result = MPI_Comm_rank(MPI_COMM_WORLD, &m_id);
    if (result != MPI_SUCCESS)
    {
        ERROR("failed to lookup MPI rank: result = " << result);
        return IOError;
    }

    return Success;
}

MpiPing::Result MpiPing::exec()
{
    int result, cores;

    if (m_id == 0)
    {
        result = MPI_Comm_size(MPI_COMM_WORLD, &cores);
        if (result != MPI_SUCCESS)
        {
            ERROR("failed to lookup MPI core count: result = " << result);
            return IOError;
        }
        NOTICE("ping " << (cores - 1) << " cores");

        // Master send a ping and receives a pong from each node
        for (int i = 1; i < cores; i++)
        {
            const Result pingResult = sendNumber(i, PingMagicNumber);
            if (pingResult != Success)
            {
                ERROR("failed to send ping to core" << i << ": result = " << (int) pingResult);
                return pingResult;
            }

            const Result pongResult = receiveNumber(i, PongMagicNumber);
            if (pongResult != Success)
            {
                ERROR("failed to receive pong from core" << i << ": result = " << (int) pongResult);
                return pongResult;
            }

            NOTICE("pong received from core" << i);
        }
    }
    else
    {
        // Slaves first receive a ping from the master, then reply with a pong
        const Result recvResult = receiveNumber(0, PingMagicNumber);
        if (recvResult != Success)
        {
            ERROR("failed to receive ping from core0: result = " << (int) recvResult);
            return recvResult;
        }

        const Result sendResult = sendNumber(0, PongMagicNumber);
        if (sendResult != Success)
        {
            ERROR("failed to send message to core0: result = " << (int) sendResult);
            return sendResult;
        }
    }

    result = MPI_Finalize();
    if (result != MPI_SUCCESS)
    {
        ERROR("failed to finalize MPI: result = " << result);
        return IOError;
    }

    return Success;
}

MpiPing::Result MpiPing::sendNumber(const Size coreId,
                                    const int number) const
{
    int buf = number;

    DEBUG("coreId = " << coreId << " number = " << Number::Hex << number);

    int result = MPI_Send(&buf, 1, MPI_INT, coreId, 0, MPI_COMM_WORLD);
    if (result != MPI_SUCCESS)
    {
        ERROR("failed to send message to core" << coreId << ": result = " << (int) result);
        return IOError;
    }

    return Success;
}

MpiPing::Result MpiPing::receiveNumber(const Size coreId,
                                       const int expectedNumber) const
{
    int buf = 0;
    MPI_Status status;

    DEBUG("coreId = " << coreId << " expectedNumber = " << Number::Hex << expectedNumber);

    int result = MPI_Recv(&buf, 1, MPI_INT, coreId, 0, MPI_COMM_WORLD, &status);
    if (result != MPI_SUCCESS)
    {
        ERROR("failed to receive message from core" << coreId << ": result = " << (int) result);
        return IOError;
    }

    if (buf != expectedNumber)
    {
        ERROR("invalid message " << Number::Hex << buf << " != " << Number::Hex << expectedNumber <<
              " received from core" << coreId);
        return IOError;
    }

    return Success;
}
