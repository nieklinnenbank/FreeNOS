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

#ifndef __KERNEL_H
#define __KERNEL_H

#include <Macros.h>
#include <Types.h>
#include <Vector.h>
#include <List.h>
#include <Singleton.h>
#include <BootImage.h>
#include <Memory.h>
#include <CoreInfo.h>

/** Forward declarations. */
class BootImageStorage;
class MemoryContext;
class SplitAllocator;
class IntController;
class Timer;

/**
 * @addtogroup kernel
 * @{
 */

/**
 * FreeNOS kernel implementation.
 */
class Kernel : public WeakSingleton<Kernel>
{
  public:

    /**
     * Result codes.
     */
    enum Result
    {
        Success,
        InvalidBootImage,
        ProcessError,
        IOError
    };

    /**
     * Constructor function.
     *
     * @param info CoreInfo structure for this core.
     */
    Kernel(CoreInfo *info);

    /**
     * Initialize heap.
     *
     * This function sets up the kernel heap for
     * dynamic memory allocation with new() and delete()
     * operators. It must be called before any object
     * is created using new().
     *
     * @return Zero on success or error code on failure.
     */
    static Error initializeHeap();

    /**
     * Get physical memory allocator.
     *
     * @return SplitAllocator object pointer
     */
    SplitAllocator * getAllocator();

    /**
     * Get the current MMU context.
     *
     * @return MemoryContext object pointer
     */
    MemoryContext * getMemoryContext();

    /**
     * Get CoreInfo.
     *
     * @return CoreInfo object pointer
     */
    CoreInfo * getCoreInfo();

    /**
     * Execute the kernel.
     */
    int run();

    /**
     * Loads the boot image.
     */
    virtual Result loadBootImage();

  private:

#if 0
    /**
     * Load a boot program.
     *
     * @param bootImage Reference to the BootImageStorage
     * @param program Reference to the BootSymbol of the program to load.
     */
    virtual Result loadBootProgram(const BootImageStorage &bootImage,
                                   const BootSymbol &program);
#endif

  protected:

    /** Physical memory allocator */
    SplitAllocator *m_alloc;

    /** CoreInfo object for this core. */
    CoreInfo *m_coreInfo;

    /** Interrupt Controller. */
    IntController *m_intControl;

    /** Timer device. */
    Timer *m_timer;
};

/**
 * @}
 */

#endif /* __KERNEL_H */
