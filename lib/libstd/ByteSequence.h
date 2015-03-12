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

#ifndef __LIBSTD_BYTESEQUENCE_H
#define __LIBSTD_BYTESEQUENCE_H

#include "Types.h"

/**
 * Object which can be read byte-wise.
 */
class ByteSequence
{
  public:

    /**
     * Class destructor.
     */
    virtual ~ByteSequence() {}
    
    /**
     * Get the size of the object.
     *
     * @return Size in bytes.
     */
    virtual Size size() const = 0;
    
    /**
     * Read an object byte-wise (e.g. for hashing).
     *
     * @param index Offset to read.
     * @return Unsigned byte.
     */
    virtual u8 valueAt(Size index) const = 0;
};

#endif /* __LIBSTD_BYTESEQUENCE_H */
