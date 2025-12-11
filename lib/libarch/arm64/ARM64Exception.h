/*
 * Copyright (C) 2025 Ivan Tan
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

#ifndef __LIBARCH_ARM64_EXCEPTION_H
#define __LIBARCH_ARM64_EXCEPTION_H

#include <Types.h>
#include <IntController.h>

/** Forward declaration */
struct CPUState;

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libarch
 * @{
 *
 * @addtogroup libarch_arm
 * @{
 */

/** Size of the ARM64 exception vector table in bytes */
#define ARM64_EX_VECTAB_SIZE   (64)

/**
 * ARM64 exception handling functionality.
 */
class ARM64Exception
{
  public:

    /**
     * ARM64 exception types.
     */
    enum ExceptionType
    {
        Sync_SP_EL0 = 0,
        IRQ_SP_EL0,
        FIQ_SP_EL0,
        SError_SP_EL0,
        Sync_SP_ELx,
        IRQ_SP_ELx,
        FIQ_SP_ELx,
        SError_SP_ELx,
        Sync_Lower_EL,
        IRQ_Lower_EL,
        FIQ_Lower_EL,
        SError_Lower_EL,
        Sync_Lower_EL_AA32,
        IRQ_Lower_EL_AA32,
        FIQ_Lower_EL_AA32,
        SError_Lower_EL_AA32,
    };

    /**
     * Result codes.
     */
    enum Result
    {
        Success,
        NotFound,
        IOError
    };

    /**
     * Function which is called when the CPU is interrupted.
     *
     * @param state State of the CPU on the moment the interrupt occurred.
     */
    typedef void Handler(struct CPUState state);

  public:

    /**
     * Constructor
     *
     */
    ARM64Exception();

    /**
     * Install an exception handler.
     *
     * @param vector Exception vector to install
     * @param handler Exception handler function to install.
     *
     * @return Result code
     */
    Result install(ExceptionType vector, Handler handler);

  private:

    /**
     * ARM64 exception vector jump table.
     *
     * The ARM64 processor will start executing at the appropriate
     * vector offset in the jump table and perform a jump to
     * the exception handler address, which it retrieves from the
     * exception handlers table.
     *
     * @see m_handlerTable;
     * @see ARM64ExceptionTable.S
     */
    Address m_vecTable;
};

/**
 * @}
 * @}
 * @}
 */

#endif /* __LIBARCH_ARM64_EXCEPTION_H */
