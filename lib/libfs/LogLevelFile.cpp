/*
 * Copyright (C) 2021 Niek Linnenbank
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

#include <Log.h>
#include <String.h>
#include "LogLevelFile.h"

LogLevelFile::LogLevelFile(const u32 inode)
    : File(inode)
{
    m_access = FileSystem::OwnerRW;
}

LogLevelFile::~LogLevelFile()
{
}

FileSystem::Result LogLevelFile::read(IOBuffer & buffer,
                                      Size & size,
                                      const Size offset)
{
    // Bounds checking
    if (offset >= 2)
    {
        size = 0;
        return FileSystem::Success;
    }
    else
    {
        // Retrieve the current log level
        const Log::Level level = Log::instance()->getMinimumLogLevel();

        // Format it as a string
        String tmp;
        tmp.set(level);
        tmp << "\n";

        // Write to the output buffer
        size = 2;
        buffer.write(*tmp, 2);

        return FileSystem::Success;
    }
}

FileSystem::Result LogLevelFile::write(IOBuffer & buffer,
                                       Size & size,
                                       const Size offset)
{
    // Bounds checking
    if (offset > 1)
    {
        size = 0;
        return FileSystem::Success;
    }
    else
    {
        char tmp[2];

        // Read input
        buffer.read(tmp, 1);
        tmp[1] = 0;

        // Convert from text to integer
        const Log::Level level = (const Log::Level) String(tmp).toLong();

        // Apply new loglevel
        Log::instance()->setMinimumLogLevel(level);

        return FileSystem::Success;
    }
}
