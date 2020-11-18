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

#ifndef __LIB_LIBRUNTIME_PROCESSCLIENT_H
#define __LIB_LIBRUNTIME_PROCESSCLIENT_H

#include <FreeNOS/User.h>
#include <FreeNOS/ProcessManager.h>
#include <Types.h>
#include <String.h>

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libruntime
 * @{
 */

/**
 * ProcessClient provides information about all processes on the local core.
 *
 * @see ProcessManager
 */
class ProcessClient
{
  public:

    /** Maximum number of processes */
    static const Size MaximumProcesses = MAX_PROCS;

    /**
     * Result codes
     */
    enum Result
    {
        Success,
        NotFound,
        IOError
    };

    /**
     * Process information
     */
    typedef struct Info
    {
        /** Process state retrieved from the kernel */
        ProcessInfo kernelState;

        /** Full command including program path */
        String command;

        /** Textual state of the process */
        String textState;
    }
    Info;

  public:

    /**
     * Get current process identifier
     *
     * @return Current Process ID
     */
    ProcessID getProcessID() const;

    /**
     * Get parent process identifier
     *
     * @return Parent Process ID
     */
    ProcessID getParentID() const;

    /**
     * Get process information by its ID.
     *
     * @param pid Process identifier of the process.
     * @param info Process information output
     *
     * @return Result code
     */
    Result processInfo(const ProcessID pid, Info &info) const;

    /**
     * Get process information by its program name
     *
     * Returns the information for the first process
     * that is running the given program.
     *
     * @param program Path to the program
     * @param info Process information output
     *
     * @return Result code
     */
    Result processInfo(const String program, Info &info) const;

    /**
     * Find a process by its program name.
     *
     * Returns the information for the first process
     * that is running the given program.
     *
     * @param program Path to the program
     *
     * @return ProcessID of the first process that matches
     *         or ANY if none found.
     */
    ProcessID findProcess(const String program) const;

  private:

    /** Our own process identifier */
    static const ProcessID m_pid;

    /** Our parent process identifier */
    static const ProcessID m_parent;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBRUNTIME_PROCESSCLIENT_H */
