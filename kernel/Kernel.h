/*
 * Copyright (C) 2009 Niek Linnenbank
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

#ifndef __KERNEL_KERNEL_H
#define __KERNEL_KERNEL_H

#include <Arch/Interrupt.h>
#include <Macros.h>
#include <Types.h>
#include <Init.h>
#include "BootImage.h"
#include "Multiboot.h"
#include "Process.h"

/**
 * @defgroup kernel kernel initialization settings.
 * @see Init.h
 * @{
 */

/** (Physical) memory must be initialized first. */
#define PMEMORY "0"

/** (Virtual) memory initialization. */
#define VMEMORY "1"

/** Constructors for C++ objects. */
#define CTOR "2"

/** Register APIHandlers. */
#define API "3"

/** Starts the scheduler. */
#define SCHEDULER "4"

/** Core kernel initialization. */
#define KERNEL "5"

/** Start of initialization routines. */
extern Address initStart;

/** Marks the end of all initialization functions. */
extern Address initEnd;

/**
 * @}
 */

/** 
 * @defgroup kernel kernel (generic)
 * @{ 
 */

/**
 * Represents the kernel core.
 */
class Kernel
{
    public:
    
        /**
         * Constructor function.
         */
        Kernel();

        /**
         * Loads the boot image.
         */
        bool loadBootImage();

        /**
         * Hooks a function to an hardware interrupt.
         * @param vec Interrupt vector to hook on.
         * @param h Handler function.
         * @param p Parameter to pass to the handler function.
         */
        virtual void hookInterrupt(int vec, InterruptHandler h, ulong p) = 0;

        /** 
         * Enable or disable an hardware interrupt (IRQ). 
         * @param vector IRQ number. 
         * @param enabled True to enable, and false to disable. 
         */
        virtual void enableIRQ(uint vector, bool enabled) = 0;

        /**
         * Create a new process.
         * @param entry Entry address of the new process.
         * @return Process pointer on success or ZERO on failure.
         */
        virtual Process * createProcess(Address entry) = 0;

    private:
    
        /**
         * Creates a new Process from a BootProcess.
         * @param image BootImage pointer loaded by the bootloader in kernel virtual memory.
         * @param imagePAddr Physical memory address of the boot image.
         * @param index Index in the BootProcess table.
         */
        void loadBootProcess(BootImage *image, Address imagePAddr, Size index);
};

/**
 * Kernel entry point.
 */
extern C void kmain();

/** Start of kernel text and data. */
extern Address kernelStart;

/** End of kernel. */
extern Address kernelEnd;

/**
 * @}
 */

#endif /* __KERNEL_KERNEL_H */
