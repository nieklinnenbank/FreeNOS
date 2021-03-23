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

#include <FreeNOS/User.h>
#include "File.h"

File::File(const u32 inode,
           const FileSystem::FileType type,
           const UserID uid,
           const GroupID gid)
    : m_inode(inode)
    , m_type(type)
    , m_uid(uid)
    , m_gid(gid)
    , m_access(FileSystem::OwnerRWX)
    , m_size(0)
{
}

File::~File()
{
}

u32 File::getInode() const
{
    return m_inode;
}

FileSystem::FileType File::getType() const
{
    return m_type;
}

FileSystem::Result File::read(IOBuffer & buffer,
                              Size & size,
                              const Size offset)
{
    return FileSystem::NotSupported;
}

FileSystem::Result File::write(IOBuffer & buffer,
                               Size & size,
                               const Size offset)
{
    return FileSystem::NotSupported;
}

FileSystem::Result File::status(FileSystem::FileStat &st)
{
    st.type     = m_type;
    st.inode    = m_inode;
    st.access   = m_access;
    st.size     = m_size;
    st.userID   = m_uid;
    st.groupID  = m_gid;

    return FileSystem::Success;
}

bool File::canRead() const
{
    return true;
}

bool File::canWrite() const
{
    return true;
}
