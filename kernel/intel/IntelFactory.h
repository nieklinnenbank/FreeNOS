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

#ifndef __INTEL_FACTORY_H
#define __INTEL_FACTORY_H

#include "ProcessFactory.h"

/**
 * Implements an x86 compatible kernel.
 */
class IntelFactory : public ProcessFactory
{
  public:

    /**
     * Create a new process.
     * @param entry Entry point of the program.
     * @return X86Process on success or ZERO on failure.
     */
    Process * createProcess(ProcessID id, Address entry);
};

#endif /* __INTEL_FACTORY_H */
