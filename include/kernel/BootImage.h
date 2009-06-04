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
#define BOOTIMAGE_MAGIC1	('N') + ('O' << 8) + ('S' << 16) + (0xff << 24)

/** Version of the boot image layout. */
#define BOOTIMAGE_REVISION	1

/** Maximum length of BootVariable keys. */
#define BOOTIMAGE_KEY		32

/** Maximum length of BootVariable values. */
#define BOOTIMAGE_VALUE		64

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
}
BootImage;

/**
 * Simple key/value entry for inside the BootImage.
 * @see BootImage
 * @note Both the key and value are not null-terminated.
 */
typedef struct BootVariable
{
    /** Key is an uninterpreted ASCII string. */
    char key[BOOTIMAGE_KEY];
    
    /** Value is an uninterpreted ASCII string. */
    char value[BOOTIMAGE_VALUE];
}
BootVariable;

#endif /* __KERNEL_BOOTIMAGE_H */
