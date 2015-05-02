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

#ifndef __ARM_ARMFACTORY_H
#define __ARM_ARMFACTORY_H

#include <FreeNOS/ProcessFactory.h>

/**
 * Create new Process objects.
 */
class ARMFactory : public ProcessFactory
{
  public:

    /**
     * Create a new ARM specific process.
     *
     * @param entry Entry address of the new process.
     * @return Process pointer on success or ZERO on failure.
     */
    virtual Process * createProcess(ProcessID id, Address entry);
};

#endif /* __ARM_ARMFACTORY_H */
