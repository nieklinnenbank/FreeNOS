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

#ifndef __SERVER_RECOVERY_RECOVERYMESSAGE_H
#define __SERVER_RECOVERY_RECOVERYMESSAGE_H

#include <Types.h>
#include <ChannelMessage.h>
#include "Recovery.h"

/**
 * @addtogroup server
 * @{
 *
 * @addtogroup recovery
 * @{
 */

/**
 * Recovery IPC message.
 */
typedef struct RecoveryMessage : public ChannelMessage
{
    ProcessID from;          /**< Source of the message */
    Recovery::Action action; /**< Action to perform. */
    Recovery::Result result; /**< Result of action. */
    ProcessID pid;           /**< Process identifier of target process. */
}
RecoveryMessage;

/**
 * @}
 * @}
 */

#endif /* __SERVER_RECOVERY_RECOVERYMESSAGE_H */
