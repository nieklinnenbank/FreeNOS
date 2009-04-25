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

#include <arch/Process.h>
#include <List.h>
#include <ListIterator.h>
#include <Macros.h>
#include <Singleton.h>
#include "Process.h"

/** 
 * @defgroup kernel kernel (generic)
 * @{ 
 */

/**
 * Responsible for deciding which Process may execute on the CPU(s).
 */
class Scheduler : public Singleton<Scheduler>
{
    public:

	/**
	 * Constructor function.
	 */
	Scheduler();
    
	/**
	 * Let the next Process run on a CPU.
	 */
	void executeNext();
	
	/**
	 * Try to execute the given process.
	 * @param p Process pointer.
	 */
	void executeAttempt(ArchProcess *p);

	/**
	 * Fetch the current process being executed.
	 * @return Pointer to the current process.
	 */
	ArchProcess *current()
	{
	    return currentProcess;
	}

        /** 
         * Fetch the previous process which was executed. 
         * @return Pointer to the previous process. 
         */
        ArchProcess *old()
        {
            return oldProcess;
        }

	/**
	 * Determines which process to run if nothing to do.
	 * @param p Process to run if no other processes ready.
	 */
	void setIdle(ArchProcess *p)
	{
	    idleProcess = p;
	    queue.remove(p);
	}

	/**
	 * Puts the given Process in the scheduler queue.
	 * @param proc Process to be scheduler later on.
	 */
	void enqueue(ArchProcess *proc);
	
	/**
	 * Removes a Process from the scheduler queue.
	 * @param proc Process which is being removed. ZERO to remove the first.
	 */
	void dequeue(ArchProcess *proc = ZERO);

    private:
    
	/**
	 * Look for the next Ready process.
	 * @return Pointer to a Ready process, or ZERO if none is Ready yet.
	 */
	ArchProcess * findNextReady();
    
	/** Contains processes waiting to be scheduled. */
	List<ArchProcess> queue;
	
	/** Points to the next process to be scheduled. */
	ListIterator<ArchProcess> queuePtr;
	
	/** Currently executing Process. */
	ArchProcess *currentProcess;

        /** Previous executing Process. */
        ArchProcess *oldProcess;
	
	/** Process to execute if nothing to do. */
	ArchProcess *idleProcess;
};

/** Scheduler instance. */
extern Scheduler *scheduler;

/**
 * @}
 */

#endif /* __ASSEMBLER__ */
#endif /* __KERNEL_SCHEDULER_H */
