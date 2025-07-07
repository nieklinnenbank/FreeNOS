.globl vecTable, handlerTable, returnFromCallSvc
//https://krinkinmu.github.io/2021/01/10/aarch64-interrupt-handling.html
//aarch64 architecuture reference manual D1.3.1.6

.macro ventry label
.align 7
b \label
.endm

.macro callHandler index
    sub sp, sp, #208

    stp x0, x1, [sp, #0]
    stp x2, x3, [sp, #16]
    stp x4, x5, [sp, #32]
    stp x6, x7, [sp, #48]
    stp x8, x9, [sp, #64]
    stp x10, x11, [sp, #80]
    stp x12, x13, [sp, #96]
    stp x14, x15, [sp, #112]
    stp x16, x17, [sp, #128]
    stp x18, x29, [sp, #144]
    mov x0, sp
    stp x30, x0, [sp, #160]

    mrs x0, esr_el1
    mrs x1, far_el1
    stp x0, x1, [sp, #176]

    mrs x0, spsr_el1
    mrs x1, elr_el1 
    stp x0, x1, [sp, #192]

    mov x0, sp
    mov x1, #\index
    //x0 == CpuState, x1 == index
    ldr x2, =handlerTable
    add x2, x2, x1, lsl #3
    ldr x1, [x2]
    cmp x1, #0
    b.eq returnFromCall\index
    blr x1

returnFromCall\index:

    ldp x0, x1, [sp, #0]
    ldp x2, x3, [sp, #16]
    ldp x4, x5, [sp, #32]
    ldp x6, x7, [sp, #48]
    ldp x8, x9, [sp, #64]
    ldp x10, x11, [sp, #80]
    ldp x12, x13, [sp, #96]
    ldp x14, x15, [sp, #112]
    ldp x16, x17, [sp, #128]
    ldp x18, x29, [sp, #144]
    ldp x30, xzr, [sp, #160]

    add sp, sp, #208

    eret    
.endm

    //align with 2048 bytes
    .align 11
vecTable:
    ventry sync_sp_el0
    ventry irq_sp_el0
    ventry fiq_sp_el0
    ventry serror_sp_el0

    //use SP_EL1 as a stack pointer, and exceptions come from same ELx, x>0
    ventry sync_sp_el1
    ventry irq_sp_el1
    ventry fiq_sp_el1
    ventry serror_sp_el1

    //exceptions come from lower EL, kernel is running in AArch64
    ventry sync_lower_el0
    ventry irq_lower_el0
    ventry fiq_lower_el0
    ventry serror_lower_el0

    //exceptions come from lower EL and AArch32
    ventry sync_lower_el0_aa32
    ventry irq_lower_el0_aa32
    ventry fiq_lower_el0_aa32
    ventry serror_lower_el0_aa32

     
returnFromCallSvc:

    ldp x30, x0, [sp, #160]
    msr sp_el0, x0
    ldp x0, x1, [sp, #192]
    msr spsr_el1, x0
    msr elr_el1, x1
    ldp x0, x1, [sp, #0]
    ldp x2, x3, [sp, #16]
    ldp x4, x5, [sp, #32]
    ldp x6, x7, [sp, #48]
    ldp x8, x9, [sp, #64]
    ldp x10, x11, [sp, #80]
    ldp x12, x13, [sp, #96]
    ldp x14, x15, [sp, #112]
    ldp x16, x17, [sp, #128]
    ldp x18, x29, [sp, #144]

    add sp, sp, #208

    eret    


sync_sp_el0:
    callHandler 0

irq_sp_el0:
    callHandler 1

fiq_sp_el0:
    callHandler 2

serror_sp_el0:
    callHandler 3

sync_sp_el1:
    callHandler 4

irq_sp_el1:
    callHandler 5

fiq_sp_el1:
    callHandler 6

serror_sp_el1:
    callHandler 7

sync_lower_el0:
    callHandler 8

irq_lower_el0:
    callHandler 9

fiq_lower_el0:
    callHandler 10

serror_lower_el0:
    callHandler 11

sync_lower_el0_aa32:
    callHandler 12

irq_lower_el0_aa32:
    callHandler 13

fiq_lower_el0_aa32:
    callHandler 14

serror_lower_el0_aa32:
    callHandler 15

handlerTable:
.fill 16, 8, 0
