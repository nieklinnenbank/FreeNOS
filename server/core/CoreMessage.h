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

#ifndef __SERVER_CORE_COREMESSAGE_H
#define __SERVER_CORE_COREMESSAGE_H

#include <Types.h>
#include <ChannelMessage.h>
#include "Core.h"

/**
 * @addtogroup server
 * @{
 *
 * @addtogroup core
 * @{
 */

/**
 * Message format for communication with the CoreServer
 *
 * @see CoreServer
 */
typedef struct CoreMessage : public ChannelMessage
{
    Core::Action action;    /**< Action to perform. */
    Core::Result result;    /**< Result code. */
    Size coreNumber;        /**< Indicates a number of cores or a specific coreId. */
    Address programAddr;    /**< Contains the virtual address of a loaded program. */
    Size programSize;       /**< Contains the size of a loaded program. */
    const char *programCmd; /**< Command-line string for a loaded program. */
}
CoreMessage;

/**
 * @}
 * @}
 */

#endif /* __SERVER_CORE_COREMESSAGE_H */
