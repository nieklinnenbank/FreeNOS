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

#include <FreeNOS/System.h>
#include "ARM64Constant.h"
#include "ARM64SecondTable.h"

/**
 * Entry inside the page table of a given virtual address.
 *
 * @param vaddr Virtual Address.
 *
 * @return Index of the corresponding page table entry.
 */
MemoryContext::Result ARM64SecondTable::map(Address virt,
                                          Address phys,
                                          Memory::Access access)
{
    u32 idx = L3_IDX(virt);
    // Check if the address is already mapped
    if (IS_PT_PAGE_TBL(m_pages[idx]))
        return MemoryContext::AlreadyExists;

    // Insert mapping
    m_pages[idx] = (phys & PAGEMASK) | PT_PAGE | flags(access);
    return MemoryContext::Success;
}

MemoryContext::Result ARM64SecondTable::unmap(Address virt)
{
    m_pages[ L3_IDX(virt) ] = PT_NONE;
    return MemoryContext::Success;
}

MemoryContext::Result ARM64SecondTable::translate(Address virt, Address *phys) const
{
    u32 idx = L3_IDX(virt);

    if (!IS_PT_PAGE_TBL(m_pages[idx]))
        return MemoryContext::InvalidAddress;

    *phys = (m_pages[idx] & PAGEMASK);
    return MemoryContext::Success;
}

MemoryContext::Result ARM64SecondTable::access(Address virt, Memory::Access *access) const
{
    u32 entry = m_pages[L3_IDX(virt)];

    if (!IS_PT_PAGE_TBL(entry))
        return MemoryContext::InvalidAddress;

    // Permissions
    *access = Memory::Readable;

    if (contain_flags(entry, PT_USER))
        *access |= Memory::User;

    if (!(contain_flags(entry, PT_RO)))
        *access |= Memory::Writable;

    // Caching
    if (contain_flags(entry, (PT_OSH | PT_DEV)))
        *access |= Memory::Device;
    else if (contain_flags(entry, PT_OSH | PT_NC))
        *access |= Memory::Uncached;
    else
        *access |= Memory::InnerCached;

    return MemoryContext::Success;
}

u32 ARM64SecondTable::flags(Memory::Access access) const
{
    u64 f = PT_KERNEL | PT_AF;

    // Permissions
    if (!(access & Memory::Executable)) f |= PT_NX;
    if ((access & Memory::User))        f |= PT_USER;
    if (!(access & Memory::Writable))   f |= PT_RO;

    // Cache
    if (access & Memory::Device)
        f |= PT_OSH | PT_DEV;
    else
        f |= PT_ISH | PT_NC; /* FIXME: Is PT_OSH appropriate? */
    return f;
}
