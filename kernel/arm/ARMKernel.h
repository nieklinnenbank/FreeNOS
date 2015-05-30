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

#ifndef __ARM_KERNEL_H
#define __ARM_KERNEL_H

#include <FreeNOS/Kernel.h>
#include <FreeNOS/Process.h>
#include <Types.h>

/** 
 * Perform a kernel trap with 1 argument.
 * @param num Unique number of the handler to execute. 
 * @return An integer. 
 */
inline ulong trapKernel1(ulong num, ulong arg1)
{
    return 0;
}

/**
 * Perform a kernel trap with 2 arguments.
 */
inline ulong trapKernel2(ulong num, ulong arg1, ulong arg2)
{
    return 0;
}

/** 
 * Perform a kernel trap with 3 arguments. 
 * @param num Unique number of the handler to execute. 
 * @return An integer. 
 */
inline ulong trapKernel3(ulong num, ulong arg1, ulong arg2, ulong arg3)
{
    return 0;
}

/** 
 * Perform a kernel trap with 4 arguments. 
 * @param num Unique number of the handler to execute. 
 * @return An integer.
 */
inline ulong trapKernel4(ulong num, ulong arg1, ulong arg2, ulong arg3,
                         ulong arg4)
{
    return 0;
}

/** 
 * Perform a kernel trap with 5 arguments. 
 * @param num Unique number of the handler to execute. 
 * @return An integer. 
 */
inline ulong trapKernel5(ulong num, ulong arg1, ulong arg2, ulong arg3,
                         ulong arg4, ulong arg5)
{
    return 0;
}

/**
 * Represents the ARM kernel implementation
 */
class ARMKernel : public Kernel
{
  public:
    
    /**
     * Constructor function.
     */
    ARMKernel(Address kernel, Size size, Size memorySize);

    /**
     * Hooks a function to an hardware interrupt.
     * @param vec Interrupt vector to hook on.
     * @param h Handler function.
     * @param p Parameter to pass to the handler function.
     */
    virtual void hookInterrupt(int vec, InterruptHandler h, ulong p);

    /** 
     * Enable or disable an hardware interrupt (IRQ). 
     * @param vector IRQ number. 
     * @param enabled True to enable, and false to disable. 
     */
    virtual void enableIRQ(uint vector, bool enabled);

    /**
     * Loads the boot image.
     */
    virtual bool loadBootImage();
};

#endif /* __ARM_KERNEL_H */
