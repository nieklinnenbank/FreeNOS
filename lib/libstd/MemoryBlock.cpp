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

#pragma clang optimize off
#pragma GCC push_options
#pragma GCC optimize ("O0")

#include "Macros.h"
#include "MemoryBlock.h"

void * MemoryBlock::set(void *dest, int ch, unsigned count)
{
    char *temp;

    for(temp = (char *) dest; count != 0; count--)
    {
        *temp++ = ch;
    }
    return (dest);
}

Size MemoryBlock::copy(void *dest, const void *src, Size count)
{
    const char *sp = (const char *)src;
    char *dp = (char *)dest;

    for(Size i = count; i != 0; i--)
        *dp++ = *sp++;

    return (count);
}

Size MemoryBlock::copy(char *dst, char *src, Size count)
{
    char *d = dst;
    const char *s = src;
    unsigned n = count;

    // Copy as many bytes as will fit
    if (n != 0) {
        while (--n != 0) {
            if ((*d++ = *s++) == '\0')
                break;
        }
    }

    // Not enough room in dst, add NUL and traverse rest of src
    if (n == 0) {
        if (count != 0)
            *d = '\0';
        while (*s++)
            ;
    }
    // Count does not include NUL
    return(s - src - 1);
}

bool MemoryBlock::compare(const void *p1, const void *p2, const Size count)
{
    const char *ch1 = (const char *) p1;
    const char *ch2 = (const char *) p2;

    for (Size i = 0; i < count; i++)
    {
        if (*ch1++ != *ch2++)
        {
            return false;
        }
    }

    return true;
}

bool MemoryBlock::compare(const char *p1, const char *p2, const Size count)
{
    const char *ch1 = (const char *) p1;
    const char *ch2 = (const char *) p2;
    Size bytes = 0;

    while (*ch1 == *ch2)
    {
        if (*ch1 == ZERO || *ch2 == ZERO)
        {
            break;
        }

        ch1++, ch2++, bytes++;

        if (count != 0 && bytes >= count)
        {
            return true;
        }
    }

    return *ch1 == *ch2;
}
