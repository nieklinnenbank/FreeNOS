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

File::File(const FileSystem::FileType type,
           const UserID uid,
           const GroupID gid)
    : m_type(type)
    , m_uid(uid)
    , m_gid(gid)
    , m_access(FileSystem::OwnerRWX)
    , m_size(0)
{
}

File::~File()
{
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

FileSystem::Result File::status(FileSystemMessage *msg)
{
    FileSystem::FileStat st;

    // Fill in the status structure
    st.type     = m_type;
    st.access   = m_access;
    st.size     = m_size;
    st.userID   = m_uid;
    st.groupID  = m_gid;

    // Copy to the remote process
    const API::Result result = VMCopy(msg->from, API::Write, (Address) &st,
                                     (Address) msg->stat, sizeof(st));
    if (result == API::Success)
    {
        return FileSystem::Success;
    }
    else
    {
        ERROR("VMCopy failed for PID " << msg->from << ": result = " << (int) result);
        return FileSystem::IOError;
    }
}
