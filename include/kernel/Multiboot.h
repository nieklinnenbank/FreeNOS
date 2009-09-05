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

#ifndef __KERNEL_MULTIBOOT_H
#define __KERNEL_MULTIBOOT_H

/**
 * @defgroup kernel kernel (generic)
 * @{
 */

/** The magic number for the Multiboot header. */
#define MULTIBOOT_HEADER_MAGIC		0x1BADB002

/** The flags for the Multiboot header. */
#define MULTIBOOT_HEADER_FLAGS		0x00000003

/** Size of the multiboot header structure. */
#define MULTIBOOT_HEADER_SIZE		52

/** The magic number passed by a Multiboot-compliant boot loader.  */
#define MULTIBOOT_BOOTLOADER_MAGIC	0x2BADB002

/* Do not include in assembler source. */
#ifndef __ASSEMBLER__

#include <Types.h>

/**
 * The symbol table for a.out.
 */
typedef struct AoutSymbolTable
{
    u32 tabSize;
    u32 strSize;
    u32 address;
    u32 reserved;
}
AoutSymbolTable;
	
/**
 * The section header table for ELF.
 */
typedef struct ElfSectionHeaderTable
{
    u32 num;
    u32 size;
    u32 address;
    u32 shndx;
}
ElfSectionHeaderTable;		

/**
 * The Multiboot information.
 */
typedef struct MultibootInfo
{
    u32 flags;
    u32 memLower;
    u32 memUpper;
    u32 bootDevice;
    u32 cmdline;
    u32 modsCount;
    u32 modsAddress;

    union
    {
        AoutSymbolTable aout;
        ElfSectionHeaderTable elf;
    };
    u32 mmapLength;
    u32 mmapAddress;
}
MultibootInfo;

/**
 * The module class.
 */
typedef struct MultibootModule
{
    u32 modStart;
    u32 modEnd;
    u32 string;
    u32 reserved;
}
MultibootModule;

/**
 * The memory map. Be careful that the offset 0 is base_addr_low
 * but no size.
 */
typedef struct MultibootMemoryMap
{
    u32 size;
    u32 baseAddressLow;
    u32 baseAddressHigh;
    u32 lengthLow;
    u32 lengthHigh;
    u32 type;
}
MultibootMemoryMap;

/** Fill in by the early boot process. */
extern MultibootInfo multibootInfo;

/**
 * @}
 */

#endif /* !__ASSEMBLER__ */
#endif /* __KERNEL_MULTIBOOT_H */
