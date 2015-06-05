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

#ifndef __ARM_CONTROL_H
#define __ARM_CONTROL_H

#include <Types.h>
#include <Macros.h>

/**
 * ARM System Control Coprocessor (CP15).
 *
 * The ARM Architecture defines a coprocessor to
 * manage various subsystems in the ARM core, including
 * virtual memory, interrupts, debugging and more.
 * It appears as coprocessor number 15 to the ARM core
 * and is accessible by the MRC (ARM Register from Coprocessor)
 * and MCR () instructions.
 *
 * @see ARM Architecture Reference Manual, page xxx
 */
class ARMControl
{
  public:

    enum Register
    {
        MainID = 0,
        TranslationTable0,
        TranslationTable1,
        TranslationTableCtrl,
    };

    /**
     * Constructor
     */
    ARMControl();

    /**
     * Destructor
     */
    virtual ~ARMControl();

    /**
     * Read a register from the CP15.
     *
     * @param reg The Register to read.
     * @return 32-bit value of the register.
     */
    u32 read(Register reg);

    /**
     * Write register to the CP15.
     *
     * @param reg The Register to write.
     * @param value 32-value to write.
     */
    void write(Register reg, u32 value);
};

#endif /* __ARM_CONTROL_H */
