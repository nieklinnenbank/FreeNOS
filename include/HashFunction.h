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

#ifndef __HASH_FUNCTION_H
#define __HASH_FUNCTION_H

#include "Types.h"
#include "Comparable.h"
#include "Assert.h"

/** Prime number used by FNV hashing. */
#define FNV_PRIME 16777619

/** Initial value of the FNV internal state. */
#define FNV_INIT  0x811c9dc5

/**
 * Compute a hash using the FNV algorithm.
 * @param key Comparable object.
 * @param mod Modulo value.
 * @return Computed hash.
 */
template <class T> Size FNVHash(Comparable<T> *key, Size mod)
{
    Size ret = FNV_INIT;
    
    assertRead(key);
    assert(mod > 0);

    for (Size i = 0; i < key->size(); i++)
    {
	ret *= FNV_PRIME;
	ret ^= key->valueAt(i);
    }
    return (ret % mod);
}

#endif /* __HASH_FUNCTION_H */
