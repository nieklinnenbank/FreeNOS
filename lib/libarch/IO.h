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

#ifndef __LIBARCH_IO_H
#define __LIBARCH_IO_H

#include <Types.h>

/**
 * Generic I/O functions.
 */
class IO
{
  public:

    enum Result
    {
        Success,
        MapFailure,
        OutOfMemory
    };

    /**
     * Constructor.
     */
    IO();

    /**
     * Get I/O base offset.
     *
     * @return Base offset to add to each I/O address.
     */
    uint getBase() const;

    /**
     * Set I/O base offset.
     *
     * @param base Offset to add to each I/O address.
     */
    void setBase(uint base);

    /**
     * Map I/O address space.
     *
     * @param phys Physical address for start of the range.
     * @param size Size of the I/O address space.
     * @return Result code.
     */
    Result map(Address phys, Size size = 4096);

  protected:

    /** I/O base offset is added to each I/O address. */
    uint m_base;
};

#endif /* __LIBARCH_IO_H */
