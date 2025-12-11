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

#include <MemoryBlock.h>
#include "ARM64Exception.h"
#include "ARM64Control.h"
#include <Log.h>
#include <String.h>

extern Address vecTable[], handlerTable[];

ARM64Exception::ARM64Exception()
    : m_vecTable((Address)vecTable)
{
    ARM64Control::write(ARM64Control::VectorBaseAddress, m_vecTable);
}

ARM64Exception::Result ARM64Exception::install(
    ARM64Exception::ExceptionType vector,
    ARM64Exception::Handler handler)
{
    handlerTable[vector] = (Address) handler;
    return Success;
}
