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

#ifndef __LIB_LIBAPP_APPLICATIONLAUNCHER_H
#define __LIB_LIBAPP_APPLICATIONLAUNCHER_H

#include <Types.h>
#include <String.h>

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libapp
 * @{
 */

/**
 * Helper class to launch an external program.
 */
class ApplicationLauncher
{
  public:

    /**
     * Result code
     */
    enum Result
    {
        Success,
        InvalidArgument,
        NotFound,
        IOError
    };

  public:

    /**
     * Constructor
     *
     * @param path Full path to the program to run
     * @param argv Array with arguments
     */
    ApplicationLauncher(const char *path, const char **argv);

    /**
     * Retrieve Process Identifier of the program
     *
     * @return ProcessID
     */
    const ProcessID getPid() const;

    /**
     * Retrieve exit code of the program.
     *
     * @return Exit code
     */
    const int getExitCode() const;

    /**
     * Runs the external program
     *
     * @return Result code
     */
    Result exec();

    /**
     * Terminate the program.
     *
     * @return Result code
     */
    Result terminate() const;

    /**
     * Wait for the program to terminate.
     *
     * @return Result code
     */
    Result wait();

  private:

    /** Absolute path to the program to run */
    const String m_path;

    /** Array with pointers to program arguments */
    const char **m_argv;

    /** PID of the DatastoreServer. */
    ProcessID m_pid;

    /** Exit code after the program has terminated. */
    int m_exitCode;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBAPP_APPLICATIONLAUNCHER_H */
