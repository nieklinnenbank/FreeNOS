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

#include <MemoryBlock.h>
#include "ARMTags.h"

ARMTags::ARMTags(Address base)
{
    m_base = base;
}

Memory::Range ARMTags::getInitRd2()
{
    Memory::Range range;
    struct ATag *tag = (struct ATag *) m_base;
    
    MemoryBlock::set(&range, 0, sizeof(range));

    while (tag)
    {
        if (tag->hdr.tag == None)
            break;

        if (tag->hdr.tag == InitRd2)
        {
            range.phys = tag->u.initrd2.start;
            range.size = tag->u.initrd2.size;
            break;
        }
        tag = ((struct ATag *)((u32 *)(tag) + (tag)->hdr.size));
    }

    return range;
}
