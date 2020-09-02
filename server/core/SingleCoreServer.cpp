/*
 * Copyright (C) 2020 Niek Linnenbank
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
#include <FreeNOS/API.h>
#include <Log.h>
#include <Factory.h>
#include "SingleCoreServer.h"

template<> CoreServer* AbstractFactory<CoreServer>::create()
{
    return new SingleCoreServer();
}

SingleCoreServer::SingleCoreServer()
    : CoreServer()
{
}

SingleCoreServer::Result SingleCoreServer::initialize()
{
    return Success;
}

Core::Result SingleCoreServer::bootCore(uint coreId, CoreInfo *info)
{
    return coreId == 0 ? Core::Success : Core::BootError;
}

Core::Result SingleCoreServer::discoverCores()
{
    return Core::Success;
}

void SingleCoreServer::waitIPI() const
{
}

Core::Result SingleCoreServer::sendIPI(uint coreId)
{
    return Core::IOError;
}
