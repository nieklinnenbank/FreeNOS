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

#ifndef __KERNEL_SCHEDULER_H
#define __KERNEL_SCHEDULER_H
#ifndef __ASSEMBLER__

#include <Vector.h>
#include <Macros.h>
#include "Process.h"

class Timer;

/**
 * @defgroup kernel kernel (generic)
 * @{
 */

/**
 * Responsible for deciding which Process may execute on the CPU(s).
 */
class Scheduler
{

  public:

    /**
     * Constructor function.
     */
    Scheduler();

    /**
     * Set timer to use
     *
     * @param timer Timer instance
     */
    void setTimer(Timer *timer);

    /**
     * Select the next process to run.
     */
    virtual Process * select(Vector<Process *> *procs, Process *idle);

  private:

    /** Contains last used index for scheduling */
    Size m_index;

    /** Points to the Timer to use for sleep timeouts */
    Timer *m_timer;
};

/**
 * @}
 */

#endif /* __ASSEMBLER__ */
#endif /* __KERNEL_SCHEDULER_H */
