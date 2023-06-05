/*
 * Copyright (C) 2017-2019 Alibaba Group Holding Limited
 */
 /******************************************************************************
 * @file     vectors.S
 * @brief    define default vector handlers. Should use with
 *           GCC for CSKY Embedded Processors
 * @version  V1.0
 * @date     28. Nove 2017
 ******************************************************************************/
#define __ASSEMBLY__

#include "irq_ctx.h"

.extern g_trap_sp

.macro restore_regs
    lw      t0, REG_INT_CTX(sp)
    csrw    mstatus, t0
    lw      t0, REG_EPC(sp)
    csrw    mepc, t0

    lw      x31, REG_X31(sp)
    lw      x30, REG_X30(sp)
    lw      x29, REG_X29(sp)
    lw      x28, REG_X28(sp)
    lw      x27, REG_X27(sp)
    lw      x26, REG_X26(sp)
    lw      x25, REG_X25(sp)
    lw      x24, REG_X24(sp)
    lw      x23, REG_X23(sp)
    lw      x22, REG_X22(sp)
    lw      x21, REG_X21(sp)
    lw      x20, REG_X20(sp)
    lw      x19, REG_X19(sp)
    lw      x18, REG_X18(sp)
    lw      x17, REG_X17(sp)
    lw      x16, REG_X16(sp)
    lw      x15, REG_X15(sp)
    lw      x14, REG_X14(sp)
    lw      x13, REG_X13(sp)
    lw      x12, REG_X12(sp)
    lw      x11, REG_X11(sp)
    lw      x10, REG_X10(sp)
    lw      x9,  REG_X9(sp)
    lw      x8,  REG_X8(sp)
    lw      x7,  REG_X7(sp)
    lw      x6,  REG_X6(sp)
    lw      x5,  REG_X5(sp)
    lw      x4,  REG_X4(sp)
    lw      x3,  REG_X3(sp)
    lw      x1,  REG_X1(sp)
    lw      x2,  REG_X2(sp)
    .endm

.macro save_regs
    la      t0, g_trap_sp
    addi    t0, t0, -XCPTCONTEXT_SIZE
    sw      x1, REG_X1(t0)
    sw      x2, REG_X2(t0)
    sw      x3, REG_X3(t0)
    sw      x4, REG_X4(t0)
    sw      x6, REG_X6(t0)
    sw      x7, REG_X7(t0)
    sw      x8, REG_X8(t0)
    sw      x9, REG_X9(t0)
    sw      x10, REG_X10(t0)
    sw      x11, REG_X11(t0)
    sw      x12, REG_X12(t0)
    sw      x13, REG_X13(t0)
    sw      x14, REG_X14(t0)
    sw      x15, REG_X15(t0)
    sw      x16, REG_X16(t0)
    sw      x17, REG_X17(t0)
    sw      x18, REG_X18(t0)
    sw      x19, REG_X19(t0)
    sw      x20, REG_X20(t0)
    sw      x21, REG_X21(t0)
    sw      x22, REG_X22(t0)
    sw      x23, REG_X23(t0)
    sw      x24, REG_X24(t0)
    sw      x25, REG_X25(t0)
    sw      x26, REG_X26(t0)
    sw      x27, REG_X27(t0)
    sw      x28, REG_X28(t0)
    sw      x29, REG_X29(t0)
    sw      x30, REG_X30(t0)
    sw      x31, REG_X31(t0)
    csrr    a0, mepc
    sw      a0, REG_EPC(t0)
    csrr    a0, mstatus
    sw      a0, REG_INT_CTX(t0)

    mv      a0, t0
    lw      t0, -4(sp)
    mv      sp, a0
    sw      t0, REG_X5(sp)
    .endm

/******************************************************************************
 * Functions:
 *     void trap(void);
 * default handler for exceptions and non-vector mode IRQs
 ******************************************************************************/
    .section .text.vectors.hook_trap_handler, "ax", %progbits
    .align  8
    .global hook_trap_handler
    .type   hook_trap_handler, %function
hook_trap_handler:
    /* Check for interrupt */
    sw      t0, -4(sp)
    csrr    t0, mcause

    /* IRQ, but in non-vector mode */
    blt     t0, x0, .Lirq

    /* Save the regs, and set a0 argument to the saved registers */
    save_regs

    jal     hook_trap_entry

    /* Check the return value and jump to the destination handler */
    bne     a0,zero, restore_and_goto_handler
    j       restore_and_return

restore_and_goto_handler:    
    restore_regs

    j       dest_trap_handler

restore_and_return:
    restore_regs

    mret

.Lirq:
    lw      t0, -4(sp)
/* MSOFT IRQ for FreeRTOS context switch
 * Config MSOFT IRQ to non-vector mode
 * tspend_handler is a weak alias to default_interrupt_handler
 */
    j       dest_trap_handler

    .size   hook_trap_handler, . - hook_trap_handler
