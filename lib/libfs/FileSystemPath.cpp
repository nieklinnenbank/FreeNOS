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

#include <String.h>
#include <MemoryBlock.h>
#include "FileSystemPath.h"

FileSystemPath::FileSystemPath(const char *path, const char separator)
    : m_separator(separator)
{
    parse(path);
}

FileSystemPath::FileSystemPath(const String &str, const char separator)
    : m_separator(separator)
{
    parse(*str);
}

FileSystemPath::~FileSystemPath()
{
    for (ListIterator<String *> i(m_path); i.hasCurrent(); i++)
        delete i.current();

    m_path.clear();
}

void FileSystemPath::parse(const char *p)
{
    const char *cur = p;

    // Skip heading separators
    while (*cur && *cur == m_separator)
    {
        cur++;
    }

    // Save parameters
    p = cur;
    m_full = cur;
    String str(p);

    // Split the path into parts
    List<String> parts = str.split(m_separator);
    for (ListIterator<String> i(parts); i.hasCurrent(); i++)
    {
        String *s = new String(i.current());
        assert(s != NULL);
        m_path.append(s);
    }

    // Create parent, if any
    if (m_path.head() && m_path.head()->next)
    {
        // Construct parent path
        for (List<String *>::Node *l = m_path.head(); l && l->next; l = l->next)
        {
            m_parent << **l->data;

            if (l->next && l->next->next)
            {
                const char tmp[] = { m_separator, ZERO };
                m_parent << tmp;
            }
        }
    }
}

const String & FileSystemPath::parent() const
{
    return m_parent;
}

const String & FileSystemPath::base() const
{
    return (*m_path.last());
}

const String & FileSystemPath::full() const
{
    return m_full;
}

const List<String *> & FileSystemPath::split()
{
    return m_path;
}

Size FileSystemPath::length() const
{
    return m_full.length();
}
