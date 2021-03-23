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
    , m_full(path)
    , m_path(m_full.split(m_separator))
    , m_base(m_path.count() > 0 ? m_path.last() : "")
    , m_parent()
{
    // Create parent, if any
    if (m_path.head() && m_path.head()->next)
    {
        const char tmp[] = { m_separator, ZERO };
        m_parent << tmp;

        // Construct parent path
        for (const List<String>::Node *l = m_path.head(); l && l->next; l = l->next)
        {
            m_parent << *l->data;

            if (l->next && l->next->next)
            {
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
    return m_base;
}

const String & FileSystemPath::full() const
{
    return m_full;
}

const List<String> & FileSystemPath::split() const
{
    return m_path;
}

Size FileSystemPath::length() const
{
    return m_full.length();
}
