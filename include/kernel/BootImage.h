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

#ifndef __KERNEL_BOOTIMAGE_H
#define __KERNEL_BOOTIMAGE_H

/** First magic byte. */
#define BOOTIMAGE_MAGIC0	('F') + ('r' << 8) + ('e' << 16) + ('e' << 24)

/** Second magic byte. */
#define BOOTIMAGE_MAGIC1	('N') + ('O' << 8) + ('S' << 16) + (0x1 << 24)

/** Version of the boot image layout. */
#define BOOTIMAGE_REVISION	1

/** Maximum length of BootVariable keys. */
#define BOOTIMAGE_KEY		32

/** Maximum length of BootVariable values. */
#define BOOTIMAGE_VALUE		64

/** Maximum length of the filesystem path in a BootProgram. */
#define BOOTIMAGE_PATH		128

/**
 * BootImage contains executable programs to be loaded at system bootup.
 */
typedef struct BootImage
{
    /** Magic numbers to detect a valid boot image. */
    u32 magic[2];
    
    /** Version of the boot image layout. */
    u8  layoutRevision;
    
    /** Checksum used to verify integrity. */
    // TODO
    
    /** Offset of the variables table. */
    u32 variablesTableOffset;
    
    /** Number of entries in the variables table. */
    u16 variablesTableCount;
    
    /** Offset of the programs table. */
    u32 programsTableOffset;
    
    /** Number of entries in the programs table. */
    u16 programsTableCount;
    
    /** Offset of the segments table. */
    u32 segmentsTableOffset;
    
    /** Number of entries in the segments table. */
    u16 segmentsTableCount;
}
BootImage;

/**
 * Simple key/value entry for inside the BootImage.
 * @see BootImage
 */
typedef struct BootVariable
{
    /** Key is an uninterpreted ASCII string. */
    char key[BOOTIMAGE_KEY];
    
    /** Value is an uninterpreted ASCII string. */
    char value[BOOTIMAGE_VALUE];
}
BootVariable;

/**
 * Program embedded in the BootImage.
 */
typedef struct BootProgram
{
    /** Path to the program. */
    char path[BOOTIMAGE_PATH];

    /** Program entry point. */
    u32 entry;
    
    /** Offset of the program segments in the segments table. */
    u32 segmentsOffset;
    
    /** Number of contiguous entries in the segment table. */
    u16 segmentsCount;
}
BootProgram;

/**
 * Program memory segment.
 */
typedef struct BootSegment
{
    /** Virtual memory address to load the segment. */
    u32 virtualAddress;
    
    /** Total size of the segment. */
    u32 size;
    
    /** Offset in the boot image of the segment contents. */
    u32 offset;
}
BootSegment;

#endif /* __KERNEL_BOOTIMAGE_H */
