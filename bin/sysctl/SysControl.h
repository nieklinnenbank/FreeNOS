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

#ifndef __BIN_SYSCTL_SYSCONTROL_H
#define __BIN_SYSCTL_SYSCONTROL_H

#include <Types.h>
#include <POSIXApplication.h>

/**
 * @addtogroup bin
 * @{
 */

/**
 * Control program for various system services.
 */
class SysControl : public POSIXApplication
{
  public:

    /**
     * Constructor
     *
     * @param argc Argument count
     * @param argv Argument values
     */
    SysControl(int argc, char **argv);

    /**
     * Destructor
     */
    virtual ~SysControl();

    /**
     * Execute the application.
     *
     * @return Result code
     */
    virtual Result exec();

  private:

    /**
     * Stop the given process by its ID.
     *
     * @param pid Process ID
     *
     * @return Result code
     */
    Result stopProcess(const ProcessID pid) const;

    /**
     * Resume the given process by its ID.
     *
     * @param pid Process ID
     *
     * @return Result code
     */
    Result resumeProcess(const ProcessID pid) const;

    /**
     * Restart the given process by its ID.
     *
     * @param pid Process ID
     *
     * @return Result code
     */
    Result restartProcess(const ProcessID pid) const;
};

/**
 * @}
 */

#endif /* __BIN_SYSCTL_SYSCONTROL_H */
