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

#ifndef __SERVER_CORE_INTELCORESERVER_H
#define __SERVER_CORE_INTELCORESERVER_H

#include "CoreServer.h"
#include <intel/IntelAPIC.h>
#include <intel/IntelMP.h>
#include <intel/IntelACPI.h>

/**
 * @addtogroup server
 * @{
 *
 * @addtogroup core
 * @{
 */

/**
 * Represents a single Core in a Central Processing Unit (CPU).
 *
 * Each core in a system will run its own instance of CoreServer.
 * CoreServers will communicate and collaborate together to implement functionality.
 *
 * @see IntelMP
 */
class IntelCoreServer : public CoreServer
{
  private:

    /** Inter-Processor-Interrupt vector number */
    static const uint IPIVector = 50;

  public:

    /**
     * Class constructor function.
     */
    IntelCoreServer();

    /**
     * Initialize the server
     *
     * @return Result code
     */
    virtual Result initialize();

    /**
     * Boot a processor core
     *
     * @param coreId Core identifier number
     * @param info CoreInfo pointer containing specific core information
     *
     * @return Result code
     */
    virtual Core::Result bootCore(uint coreId, CoreInfo *info);

    /**
     * Discover processor cores
     *
     * @return Result code
     */
    virtual Core::Result discoverCores();

    /**
     * Wait for Inter-Processor-Interrupt
     */
    virtual void waitIPI() const;

    /**
     * Send Inter-Processor-Interrupt
     *
     * @param coreId Core identifier number
     *
     * @return Result code
     */
    virtual Core::Result sendIPI(uint coreId);

  private:

    IntelAPIC m_apic;
    IntelMP m_mp;
    IntelACPI m_acpi;
};

/**
 * @}
 * @}
 */

#endif /* __SERVER_CORE_INTELCORESERVER_H */
