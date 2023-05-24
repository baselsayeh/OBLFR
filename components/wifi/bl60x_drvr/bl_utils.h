/*
 * Copyright (c) 2016-2023 Bouffalolab.
 *
 * This file is part of
 *     *** Bouffalolab Software Dev Kit ***
 *      (see www.bouffalolab.com).
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of Bouffalo Lab nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __RWNX_UTILS_H__
#define __RWNX_UTILS_H__
#include <stdint.h>
#define CFG_CHIP_BL808 1
#define u32 uint32_t
#define u8 uint8_t
//#include "bl_defs.h"
#ifdef CONFIG_RWNX_DBG

/**
 ****************************************************************************************
 *
 * @file bl_utils.h
 * Copyright (C) Bouffalo Lab 2016-2018
 *
 ****************************************************************************************
 */

#define RWNX_DBG os_printf
#else
#define RWNX_DBG(a...) do {} while (0)
#endif

#define RWNX_FN_ENTRY_STR ">>> %s()\r\n", __func__
#define RWNX_FN_LEAVE_STR "<<< %s()\r\n", __func__

#define RWNX_RXBUFF_PATTERN     (0xCAFEFADE)
#define RWNX_RXBUFF_VALID_IDX(idx) ((idx) < RWNX_RXBUFF_MAX)
/* Used to ensure that hostid set to fw is never 0 */
#define RWNX_RXBUFF_IDX_TO_HOSTID(idx) ((idx) + 1)
#define RWNX_RXBUFF_HOSTID_TO_IDX(hostid) ((hostid) - 1)
#define RWNX_RXBUFF_DMA_ADDR_GET(skbuff)                \
    skbuff->payload

/*
 * Structure used to store information regarding E2A msg buffers in the driver
 */
struct bl_e2amsg_elem {
    struct ipc_e2a_msg *msgbuf_ptr;
    u32 dma_addr;//XXX only for 32-bit addr
};

/*
 * Structure used to store information regarding Debug msg buffers in the driver
 */
struct bl_dbg_elem {
    struct ipc_dbg_msg *dbgbuf_ptr;
    u32 dma_addr;
};

/*
 * Structure used to store information regarding E2A radar events in the driver
 */
struct bl_e2aradar_elem {
    struct radar_pulse_array_desc *radarbuf_ptr;
    u32 dma_addr;
};

int bl_ipc_init(struct bl_hw *bl_hw, struct ipc_shared_env_tag *ipc_shared_mem);
uint32_t* bl_utils_pbuf_alloc(void);
void bl_utils_pbuf_free(uint32_t *p);
int bl_utils_idx_lookup(struct bl_hw *bl_hw, uint8_t *mac);

//
int bl_supplicant_init(void);

//reg_Access
//#define IPC_REG_BASE_ADDR             0x00800000
/* Macros for IPC registers access (used in reg_ipc_app.h) */
/*#define REG_IPC_APP_RD(env, INDEX)                                      \
    (*(volatile u32*)((u8*)env + IPC_REG_BASE_ADDR + 4*(INDEX)))

#define REG_IPC_APP_WR(env, INDEX, value)                               \
    (*(volatile u32*)((u8*)env + IPC_REG_BASE_ADDR + 4*(INDEX)) = value)*/



#include "bl60x_wifi_driver/reg_ipc_app.h"
#include "bl60x_wifi_driver/ipc_shared.h"
//Stuff from reg_ipc_app.h
/*#ifdef CFG_CHIP_BL602
#define REG_WIFI_REG_BASE         0x44000000
#endif
#ifdef CFG_CHIP_BL808*/
//#define REG_WIFI_REG_BASE         0x24000000
/*#endif
#ifdef CFG_CHIP_BL606P
#define REG_WIFI_REG_BASE         0x24000000
#endif*/

/*#define IPC_EMB2APP_UNMASK_CLEAR_ADDR   0x12000010
#define IPC_EMB2APP_UNMASK_CLEAR_OFFSET 0x00000010
#define IPC_EMB2APP_UNMASK_CLEAR_INDEX  0x00000004
#define IPC_EMB2APP_UNMASK_CLEAR_RESET  0x00000000

static inline void ipc_emb2app_unmask_clear(u32 value)
{
    REG_IPC_APP_WR(REG_WIFI_REG_BASE, IPC_EMB2APP_UNMASK_CLEAR_INDEX, value);
}*/
//From ipc_shared.h
/*
#define CO_BIT(pos) (1U<<(pos))

#define NX_TXQ_CNT          4
/// IPC TX descriptor interrupt mask
#define IPC_IRQ_A2E_TXDESC          0xFF00

#define IPC_IRQ_A2E_TXDESC_FIRSTBIT (8)
#define IPC_IRQ_A2E_RXBUF_BACK      CO_BIT(5)
#define IPC_IRQ_A2E_RXDESC_BACK     CO_BIT(4)

#define IPC_IRQ_A2E_MSG             CO_BIT(1)
#define IPC_IRQ_A2E_DBG             CO_BIT(0)

#define IPC_IRQ_A2E_ALL             (IPC_IRQ_A2E_TXDESC|IPC_IRQ_A2E_MSG|IPC_IRQ_A2E_DBG)

// IRQs from emb to app
#define IPC_IRQ_E2A_TXCFM_POS   7

#define IPC_IRQ_E2A_TXCFM       ((1 << NX_TXQ_CNT) - 1 ) << IPC_IRQ_E2A_TXCFM_POS

#define IPC_IRQ_E2A_RADAR       CO_BIT(6)
#define IPC_IRQ_E2A_TBTT_SEC    CO_BIT(5)
#define IPC_IRQ_E2A_TBTT_PRIM   CO_BIT(4)
#define IPC_IRQ_E2A_RXDESC      CO_BIT(3)
#define IPC_IRQ_E2A_MSG_ACK     CO_BIT(2)
#define IPC_IRQ_E2A_MSG         CO_BIT(1)
#define IPC_IRQ_E2A_DBG         CO_BIT(0)

#define IPC_IRQ_E2A_ALL         ( IPC_IRQ_E2A_TXCFM         \
                                | IPC_IRQ_E2A_RXDESC        \
                                | IPC_IRQ_E2A_MSG_ACK       \
                                | IPC_IRQ_E2A_MSG           \
                                | IPC_IRQ_E2A_DBG           \
                                | IPC_IRQ_E2A_TBTT_PRIM     \
                                | IPC_IRQ_E2A_TBTT_SEC      \
                                | IPC_IRQ_E2A_RADAR)
*/
#endif
