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

#include <arch/Memory.h>
#include <Types.h>
#include <Macros.h>
#include <Init.h>
#include <PageAllocator.h>
#include <ListAllocator.h>
#include "runtime.h"

extern C int __cxa_atexit(void (*func) (void *), void * arg, void * dso_handle)
{
    return (0);
}

extern C void __cxa_pure_virtual()
{
}

extern C void __dso_handle()
{
}

extern C void SECTION(".entry") _entry() 
{
    void (**ctor)(), (**dtor)();
    char *argv[] = {"main", ZERO };

    /* Setup heap. */
    PageAllocator pa(PAGESIZE * 8), *p;
    ListAllocator *li;
    Address heapAddr = pa.getHeapStart(), heapOff;
    
    /* Allocate instance copy on vm pages itself. */
    p  = new (heapAddr) PageAllocator(&pa);
    li = new (heapAddr + sizeof(PageAllocator)) ListAllocator();
    
    /* Point to the next free space. */
    heapOff   = sizeof(PageAllocator) + sizeof(ListAllocator);
    heapAddr += heapOff;

    /* Setup the userspace heap allocator region. */
    li->region(heapAddr, (PAGESIZE * 4) - heapOff);
    li->setParent(p);

    /* Set default allocator. */
    Allocator::setDefault(li);

    /* Run constructors. */
    for (ctor = &CTOR_LIST; ctor && *ctor; ctor++)
    {
        (*ctor)();
    }
    /* Pass control to the program. */
    main(1, argv);

    /* Run destructors. */
    for (dtor = &DTOR_LIST; dtor && *dtor; dtor++)
    {
        (*dtor)();
    }
}
