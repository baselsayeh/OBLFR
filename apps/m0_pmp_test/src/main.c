/**
 * @file main.c
 * @brief
 *
 * Copyright (c) 2022 Bouffalolab team
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 */

#include <stdio.h>
#include <board.h>
#include <bflb_mtimer.h>
#include "sdkconfig.h"

#define DBG_TAG "MAIN"
#include <log.h>

//Needed for PMP
#include "rv_pmp.h"

void iiinnniiittt_pmp() {
    int ret;

    //const pmp_config_entry_t pmp_entry_tab[6] = {
    const pmp_config_entry_t pmp_entry_tab[] = {
        /* no access */
        //[0] = {
        {
            //.entry_flag = ENTRY_FLAG_ADDR_NAPOT | ENTRY_FLAG_M_MODE_L,
            .entry_flag = ENTRY_FLAG_ADDR_NAPOT,
            .entry_pa_base = 0x53FC0000,
            .entry_pa_length = PMP_REG_SZ_128B,
        },
        {
            //.entry_flag = ENTRY_FLAG_ADDR_NAPOT | ENTRY_FLAG_M_MODE_L, //Omitting ENTRY_FLAG_M_MODE_L will make it not fire an exception for m mode
            .entry_flag = ENTRY_FLAG_ADDR_NAPOT,
            .entry_pa_base = 0x53FA0000,
            .entry_pa_length = PMP_REG_SZ_128B,
        },

        //Covers the rest as RWX
        {
            .entry_flag = ENTRY_FLAG_ADDR_NAPOT | ENTRY_FLAG_PERM_R|ENTRY_FLAG_PERM_W|ENTRY_FLAG_PERM_X,
            .entry_pa_base = 0x0,
            .entry_pa_length = PMP_REG_SZ_1G,
        },
        {
            .entry_flag = ENTRY_FLAG_ADDR_NAPOT | ENTRY_FLAG_PERM_R|ENTRY_FLAG_PERM_W|ENTRY_FLAG_PERM_X,
            .entry_pa_base = 0x40000000,
            .entry_pa_length = PMP_REG_SZ_1G,
        },
        {
            .entry_flag = ENTRY_FLAG_ADDR_NAPOT | ENTRY_FLAG_PERM_R|ENTRY_FLAG_PERM_W|ENTRY_FLAG_PERM_X,
            .entry_pa_base = 0x80000000,
            .entry_pa_length = PMP_REG_SZ_1G,
        },
        {
            .entry_flag = ENTRY_FLAG_ADDR_NAPOT | ENTRY_FLAG_PERM_R|ENTRY_FLAG_PERM_W|ENTRY_FLAG_PERM_X,
            .entry_pa_base = 0xC0000000,
            .entry_pa_length = PMP_REG_SZ_1G,
        },
    };
    ret = rvpmp_init(pmp_entry_tab, sizeof(pmp_entry_tab) / sizeof(pmp_config_entry_t));
    LOG_I("PMP Init ret: %d\r\n", ret);
}

#include "exc.c"

void __activate_mprv() {
    __mstatus_set_mprv(1);
    __mstatus_set_mpp(0x03);

    __asm__ volatile ("csrw mepc, ra");
    __asm__ volatile ("mret");
}

int main(void)
{
    board_init();

    LOG_I("Hello world!\r\n");

    //Init the PMP
    LOG_I("PMP Init ...\r\n");
    iiinnniiittt_pmp();
    LOG_I("PMP Init Done ...\r\n");
    //

    //Activate MPRV
    LOG_I("Activating MPRV...\r\n");
    //__pmp_set_mprv(1);
    __activate_mprv();
    LOG_I("MPRV Acrivated!\r\n");

    //Tese the pmp by writing to undefined area
    LOG_I("Testing PMP ...\r\n");
    ((uint32_t *)0x53FC0000)[0] = 0xDEADBEEF;
    ((uint32_t *)0x53FA0000)[0] = 0xF00DBABE;
    LOG_I("PMP Testing Done ...\r\n");
    //

    printf("PMP val: %08x, %08x\r\n", ((uint32_t *)0x53FC0000)[0], ((uint32_t *)0x53FA0000)[0]);

    while (1)
    {
        LOG_I("Waiting for inturrupts\r\n");
        __asm("wfi");
    }
}
