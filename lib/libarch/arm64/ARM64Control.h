/*
 * Copyright (C) 2025 Ivan Tan
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

#ifndef __ARM64_CONTROL_H
#define __ARM64_CONTROL_H

#include <Types.h>
#include <Macros.h>

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libarch
 * @{
 *
 * @addtogroup libarch_arm
 * @{
 */

/**
 * ARM64 System Control Coprocessor (CP15).
 *
 * The ARM64 Architecture defines a coprocessor to
 * manage various subsystems in the ARM64 core, including
 * virtual memory, interrupts, debugging and more.
 * It appears as coprocessor number 15 to the ARM64 core
 * and is accessible by the MRC (ARM64 Register from Coprocessor)
 * and MCR () instructions.
 *
 * @see ARM64 Architecture Reference Manual, page xxx
 */
namespace ARM64Control
{
    /**
     * System Control Registers
     */
    enum Register
    {
        MainID = 0,
        CoreID,
        SystemControl,  //checked
        AuxControl,
        DomainControl,
        TranslationTable0, //checked
        TranslationTable1, //checked
        TranslationTableCtrl,//checked
        CacheClear,
        DataCacheClean,
        FlushPrefetchBuffer,
        InstructionCacheClear,
        InstructionTLBClear,
        DataTLBClear,
        UnifiedTLBClear,
        UserProcID,
        InstructionFaultAddress,
        InstructionFaultStatus,
        DataFaultAddress,
        DataFaultStatus,
        SystemFrequency,
        PhysicalTimerValue,
        PhysicalTimerControl,
        PhysicalTimerCount,
        //aarch64 new registers
        MemoryModelFeature,
        MemoryAttrIndirection,
        VectorBaseAddress,
        DAIF,
    };


    /**
     * Aux Control flags.
     */
    enum AuxControlFlags
    {
        DisablePageColoring = (1 << 6),
        SMPBit              = (1 << 6)
    };

    /**
     * Domain Control flags.
     */
    enum DomainControlFlags
    {
        DomainClient  = 1,
        DomainManager = 3
    };

    /**
     * Read a register from the CP15.
     *
     * @param reg The Register to read.
     *
     * @return 32-bit value of the register.
     */
    u64 read(Register reg);

    /**
     * Write register to the CP15.
     *
     * @param reg The Register to write.
     * @param value 32-value to write.
     */
    void write(Register reg, u64 value);

    /**
     * Set auxillary flags in CP15.
     *
     * @param flags Auxillary flags to set
     */
    void set(AuxControlFlags flags);

    /**
     * Set domain control flags in CP15.
     *
     * @param flags New domain control flags
     */
    void set(DomainControlFlags flags);
};

/**
 * @}
 * @}
 * @}
 */

/* Macros for registers
 *
 */

//AArch64 Memory Model Feature Register 0
//https://developer.arm.com/documentation/ddi0601/2025-03/AArch64-Registers/ID-AA64MMFR0-EL1--AArch64-Memory-Model-Feature-Register-0
#define PA_RANGE(r) ((r)&0xF)       //Physical Address range supported.
#define TGRAN4(r)   (((r)>>28)&0xF) //Indicates support for 4KB memory translation granule size.

#define tlb_invalidate(virt) \
({ \
    asm volatile ("dsb ishst\n"\
                  "tlbi vaae1is, %0\n" \
                  "dsb ish\n" \
                  "isb" : : "r" (virt>>12UL) );\
})

/**
 * Instruction Synchronisation Barrier (ARMv7 and above)
 */
#define isb() { asm volatile ("isb" ::: "memory"); }

/**
 * Data Memory Barrier
 *
 * Ensures that all memory transactions are complete when
 * the next instruction runs. If the next instruction is not
 * a memory instruction, it is allowed to run out of order.
 * The DMB provides slightly looser memory barrier than DSB on ARM.
 */
#define dsb(type) { asm volatile ("dsb "#type ::: "memory"); }

/**
 * Data Memory Barrier
 *
 * Ensures that all memory transactions are complete when
 * the next instruction runs. If the next instruction is not
 * a memory instruction, it is allowed to run out of order.
 * The DMB provides slightly looser memory barrier than DSB on ARM.
 */
//FIXME: sy isn't appropriate for all scenarioes
#define dmb()  { asm volatile ("dmb sy" ::: "memory"); }

#define enable_interrupt() { asm volatile ("msr daifclr, #2") ; }
#define disable_interrupt() { asm volatile ("msr daifset, #2") ; }

/**
 * Reads the CPU's timestamp counter.
 *
 * @return 64-bit integer.
 */
#define timestamp() 0

/**
 * Reboot the system
 */
#define cpu_reboot()

/**
 * Shutdown the machine via ACPI.
 *
 * @note We do not have ACPI yet. Shutdown now has a bit naive implementation.
 * @see http://forum.osdev.org/viewtopic.php?t=16990
 */
#define cpu_shutdown()

/**
 * Puts the CPU in a lower power consuming state.
 */
#define idle() \
    asm volatile ("wfi")

/**
 * System Control flags.
 */
namespace SystemControlFlags {
    enum {
        MMUEnabled          = (1 << 0),
        AlignCheckEnabled   = (1 << 1),
        Stage1Cache         = (1 << 2),
        SA                  = (1 << 2),  // SP Alignment check enable
        SA0                 = (1 << 3),  // SP Alignment check enable for EL0
        InstrCache          = (1 << 12),
        WXN                 = (1 << 19), // Write permission implies XN (Execute-never).
        E0E                 = (1 << 24), // Endianness of data accesses at EL0.
        EE                  = (1 << 25), //Endianness of data accesses at EL1, and stage 1 translation table walks in the EL1&0 translation regime.
    };
};

/** 
 * Contains all the CPU registers.
 */
typedef struct CPUState
{
    u64 x0, x1, x2, x3, x4, x5;
    u64 x6, x7, x8, x9, x10, x11;
    u64 x12, x13, x14, x15, x16;
    u64 x17, x18, fp, lr, sp; //sp_el0
    u64 esr, far, cpsr, pc;
}
ALIGN(8) CPUState;

#define exception_code(esr) (((esr)>>26)&0x3f)

#endif /* __ARM64_CONTROL_H */
