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

#include <String.h>
#include <ListIterator.h>
#include <MemoryBlock.h>
#include "Directory.h"

Directory::Directory(const u32 inode)
    : File(inode, FileSystem::DirectoryFile)
{
    insert(FileSystem::DirectoryFile, ".");
    insert(FileSystem::DirectoryFile, "..");
}

Directory::~Directory()
{
    for (ListIterator<Dirent *> i(entries); i.hasCurrent(); i++)
        delete i.current();

    entries.clear();
}

FileSystem::Result Directory::read(IOBuffer & buffer,
                                   Size & size,
                                   const Size offset)
{
    Size bytes = 0;

    // Loop our list of Dirents
    for (ListIterator<Dirent *> i(&entries); i.hasCurrent(); i++)
    {
        // Can we read another entry?
        if (bytes + sizeof(Dirent) <= size)
        {
            buffer.write(i.current(), sizeof(Dirent), bytes);
            bytes += sizeof(Dirent);
        }
        else break;
    }

    // Report results
    size = bytes;
    return FileSystem::Success;
}

File * Directory::lookup(const char *name)
{
    return ZERO;
}

void Directory::insert(FileSystem::FileType type, const char *name)
{
    Dirent *d;

    // Only insert if not already in
    if (!get(name))
    {
        // Create an fill entry object
        d = new Dirent;
        assert(d != NULL);
        MemoryBlock::copy(d->name, (char *)name, DIRENT_LEN);
        d->type = type;
        entries.append(d);
        m_size += sizeof(*d);
    }
}

void Directory::remove(const char *name)
{
    const String str(name, false);

    for (ListIterator<Dirent *> i(&entries); i.hasCurrent(); i++)
    {
        if (str.compareTo(i.current()->name) == 0)
        {
            delete i.current();
            i.remove();
            m_size -= sizeof(Dirent);
            return;
        }
    }
}

void Directory::clear()
{
    for (ListIterator<Dirent *> i(entries); i.hasCurrent(); i++)
        delete i.current();

    entries.clear();
}

Dirent * Directory::get(const char *name)
{
    const String str(name, false);

    for (ListIterator<Dirent *> i(&entries); i.hasCurrent(); i++)
    {
        if (str.compareTo(i.current()->name) == 0)
        {
            return i.current();
        }
    }
    return (Dirent *) ZERO;
}
