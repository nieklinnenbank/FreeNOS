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

#include <DeviceServer.h>
#include "ATAController.h"
#include <Error.h>
#include <Types.h>
#include <stdlib.h>
#include <syslog.h>

int main(int argc, char **argv)
{
    DeviceServer server("ata", CharacterDeviceFile);

    /* Open the system log. */
    openlog("ATA", LOG_PID | LOG_CONS, LOG_USER);

    /* 
     * Start serving requests. 
     */
    server.add(new ATAController);
    return server.run();
}

ATAController::ATAController()
{
}

Error ATAController::initialize()
{
    ATADrive *drive;

    /*
     * Request ATA Control I/O port.
     */
    ProcessCtl(SELF, AllowIO, ATA_BASE_CTL0);

    /*
     * Request ATA Command I/O ports.
     */
    for (Size i = 0; i <= ATA_REG_CMD; i++)
    {
	ProcessCtl(SELF, AllowIO, ATA_BASE_CMD0 + i);
    }
    /* Detect ATA Controller. */
    if (ReadByte(ATA_BASE_CMD0 + ATA_REG_STATUS) == 0xff)
    {
	exit(EXIT_FAILURE);
    }
    pollReady(true);
    
    /* Attempt to detect first drive. */
    WriteByte(ATA_BASE_CMD0 + ATA_REG_SELECT, ATA_SEL_MASTER);
    pollReady(true);
    WriteByte(ATA_BASE_CMD0 + ATA_REG_CMD,    ATA_CMD_IDENTIFY);

    switch (ReadByte(ATA_BASE_CMD0 + ATA_REG_STATUS))
    {
	case 0:
	    syslog(LOG_INFO, "No ATA drive(s) detected");
	    break;
	
	default:
	    /* Wait until the command completed. */
	    pollReady();

	    /* Allocate a new drive. */
	    drive = new ATADrive;
	    drives.append(drive);

	    /* Read IDENTIFY data. */
	    for (int i = 0; i < 256; i++)
	    {
		((u16 *) &drive->identity)[i] = ReadWord(ATA_BASE_CMD0 + ATA_REG_DATA);
	    }
	    /* Fixup ASCII bytes. */
	    IDENTIFY_TEXT_SWAP(drive->identity.firmware, 8);
	    IDENTIFY_TEXT_SWAP(drive->identity.serial, 20);
	    IDENTIFY_TEXT_SWAP(drive->identity.model, 40);

	    /* Print out information. */
	    syslog(LOG_INFO, "ATA drive detected: SERIAL=%20s FIRMWARE=%8s "
			     "MODEL=%40s MAJOR=%x MINOR=%x SECTORS=%x",
			      drive->identity.serial,
			      drive->identity.firmware,
			      drive->identity.model,
			      drive->identity.majorRevision,
			      drive->identity.minorRevision,
			      drive->identity.sectors28);
	    break;
    }
    return ESUCCESS;
}

Error ATAController::read(s8 *buffer, Size size, Size offset)
{
    u8 sectors = CEIL(size, 512);
    u16 block[256];
    u32 lba     = offset / 512;
    Size result = 0;

    /* Verify LBA. */
    if (drives.isEmpty() || drives.first()->identity.sectors28 < lba)
    {
	return EIO;
    }
    /* Perform ATA Read Command. */
    WriteByte(ATA_BASE_CMD0 + ATA_REG_SELECT, ATA_SEL_MASTER_28);
    WriteByte(ATA_BASE_CMD0 + ATA_REG_COUNT,  sectors);
    WriteByte(ATA_BASE_CMD0 + ATA_REG_ADDR0,  (lba) & 0xff);
    WriteByte(ATA_BASE_CMD0 + ATA_REG_ADDR1,  (lba >> 8) & 0xff);
    WriteByte(ATA_BASE_CMD0 + ATA_REG_ADDR2,  (lba >> 16) & 0xff);
    WriteByte(ATA_BASE_CMD0 + ATA_REG_CMD, ATA_CMD_READ);
    
    /*
     * Read out all requested sectors.
     */
    while(result < size)
    {
	/* Poll the status register. */
	pollReady(true);

        /* Read out bytes. */
	for (int i = 0; i < 256; i++)
	{
	    block[i] = ReadWord(ATA_BASE_CMD0 + ATA_REG_DATA);
	}
	/* Calculate maximum bytes. */
	Size bytes = (size - result) < 512 - (offset % 512) ?
		     (size - result) : 512 - (offset % 512);

	/* Copy to buffer. */
	memcpy(buffer + result, ((u8 *)block) + (offset % 512), bytes);
	
	/* Update state. */
	result += bytes;
	offset += bytes;
    }
    return result;
}

Error ATAController::interrupt(Size vector)
{
    syslog(LOG_INFO, "ATA interrupted on IRQ %u", vector);
    return ESUCCESS;
}

void ATAController::pollReady(bool noData)
{
    while (true)
    {
	u8 status = ReadByte(ATA_BASE_CMD0 + ATA_REG_STATUS);
	
	if (!(status & ATA_STATUS_BUSY) &&
	     (status & ATA_STATUS_DATA || noData))
	{
	    break;
	}
    }
}
