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

#ifndef __X86_CPU_H
#define __X86_CPU_H

/**   
 * @defgroup x86kernel kernel (x86)
 * @{   
 */

/** Intel is little endian. */
#define CPU_LITTLE_ENDIAN	1

#if !defined(__HOST__)

/** Paged Mode. */
#define CR0_PG		0x80000000

/** Timestamp Counter Disable. */
#define CR4_TSD		0x00000004

/** Kernel Code Segment. */
#define KERNEL_CS       1 
#define KERNEL_CS_SEL   0x8 

/** System Data Segment. */
#define KERNEL_DS       2 
#define KERNEL_DS_SEL   0x10 
    
/** User Code Segment. */
#define USER_CS         3 
#define USER_CS_SEL     (0x18 | 3) 
        
/** User Data Segment. */
#define USER_DS         4 
#define USER_DS_SEL     (0x20 | 3) 

/** User TSS. */
#define USER_TSS        5 
#define USER_TSS_SEL    0x28 

#ifndef __ASSEMBLER__

#include <Types.h>
#include <Macros.h>

/**
 * Retrieve the IRQ number from CPUState.
 * @return IRQ number.
 */
#define IRQ_REG(state) \
    ((state)->vector - 0x20)

/**
 * Reads the CPU's timestamp counter.
 * @return 64-bit integer.
 */
#define timestamp() \
    ({ \
	u64 low = 0, high = 0; \
	asm volatile ("rdtsc\n" \
		      "movl %%eax, %0\n" \
		      "movl %%edx, %1\n" : "=r"(low), "=r"(high)); \
	(u64) (high << 32) | (low); \
    })

/**
 * Reboot the system (by sending the a reset signal on the keyboard I/O port)
 */
#define reboot() \
    outb(0x64, 0xfe)

/**
 * Shutdown the machine via ACPI.
 * @note We do not have ACPI yet. Shutdown now has a bit naive implementation.
 * @see http://forum.osdev.org/viewtopic.php?t=16990
 */
#define shutdown() \
    outw(0xB004, 0x0 | 0x2000)

/**  
 * Puts the CPU in a lower power consuming state. 
 */
#define idle() \
    asm volatile ("hlt");

/**
 * Loads the Task State Register (LTR) with the given segment.
 * @param sel TSS segment selector.
 */
#define ltr(sel) \
({ \
    u16 tr = sel; \
    asm volatile ("ltr %0\n" :: "r"(tr)); \
})

/**  
 * Read a byte from a port.  
 * @param port The I/O port to read from.  
 * @return A byte read from the port.  
 */
#define inb(port) \
({ \
    unsigned char b; \
    asm volatile ("inb %%dx, %%al" : "=a" (b) : "d" (port)); \
    b; \
})

/**
 * Read a word from a port.
 * @param port The I/O port to read from.
 * @return Word read from the port.
 */
#define inw(port) \
({ \
    unsigned short w; \
    asm volatile ("inw %%dx, %%ax" : "=a" (w) : "d" (port)); \
    w; \
})

/**  
 * Output a byte to a port.  
 * @param port Port to write to.  
 * @param byte The byte to output.  
 */
#define outb(port,byte) \
    asm volatile ("outb %%al,%%dx"::"a" (byte),"d" (port))

/**
 * Output a word to a port.
 * @param port Port to write to.
 * @param byte The word to output.
 */
#define outw(port,word) \
    asm volatile ("outw %%ax,%%dx"::"a" (word),"d" (port))

/** 
 * Output a long to a I/O port. 
 * @param port Target I/O port. 
 * @param l The long 32-bit number to output.
 */
#define outl(port,l) \
    asm volatile ("outl %%eax,%%dx"::"a" (l),"d" (port))

/** I/O bitmap. */
extern Address kernelioBitMap[1024];

#ifdef CPP

/** 
 * Intel's Task State Segment. 
 */
typedef struct TSS
{
    /**
     * (Un)mask the appropriate bit to (dis)allow direct I/O.
     * @param port I/O port.
     * @param enabled Allow or disallow a port.
     * @param bitmap Pointer to the bitmap.
     */
    void setPort(u16 port, bool enabled, u8 *bitmap)
    {
	if (enabled)
	    bitmap[(port / 8)] &= ~(1 << (port % 8));
	else
	    bitmap[(port / 8)] |=  (1 << (port % 8));
    }

    u32 backlink;
    u32 esp0, ss0;
    u32 esp1, ss1;
    u32 esp2, ss2;
    u32 cr3;
    u32 eip;
    u32 eflags;
    u32 eax,ecx,edx,ebx;
    u32 esp, ebp;
    u32 esi, edi;
    u32 es, cs, ss, ds, fs, gs;
    u32 ldt;
    u32 bitmap;
}
TSS;

/** 
 * Segment descriptor used in the GDT. 
 */
typedef struct Segment
{
    u32 limitLow:16;
    u32 baseLow:16;
    u32 baseMid:8;
    u32 type:5;
    u32 privilege:2;
    u32 present:1;
    u32 limitHigh:4;
    u32 granularity:4;
    u32 baseHigh:8;
}
Segment;

/** 
 * Contains all the CPU registers. 
 */
typedef struct CPUState
{
    /* Segments. */
    u32 gs, fs, es, ds, ss0;

    /* By pusha */
    u32 edi, esi, ebp, esp0, ebx, edx, ecx, eax;

    /* Vector/error arguments. */
    u32 vector, error;

    /* Pushed by processor. */
    u32 eip, cs, eflags, esp3, ss3;
}
CPUState;

/** Global Descriptor Table. */
extern Segment gdt[];

/** Task State Segment. */
extern TSS kernelTss;

#endif /* CPP */

/**
 * @}
 */

#endif /* __ASSEMBLER__ */
#endif /* !defined(__HOST__) && !defined(CPP) */
#endif /* __X86_CPU_H */
