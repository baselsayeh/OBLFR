/**
 * @file wifi.c
 * @brief
 *
 * Copyright (c) 2023 Bouffalolab team
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

/*#include <stdio.h>
#include <board.h>
#include <bflb_mtimer.h>
#include "sdkconfig.h"

//Free RTOS
#include "FreeRTOSConfig.h"
#include <FreeRTOS.h>
#include <task.h>

#define DBG_TAG "WIFI"
#include <log.h>*/

#include <bl808.h>
#include <bl808_common.h>
#include <bl808_glb.h>
#include <ipc_reg.h>
#include <bflb_clock.h>
#include <bflb_gpio.h>

#define GLB_WIFI_CFG0_OFFSET                                    (0x3B0)
#define GLB_WIFI_MAC_CORE_DIV                                   GLB_WIFI_MAC_CORE_DIV
#define GLB_WIFI_MAC_CORE_DIV_POS                               (0U)
#define GLB_WIFI_MAC_CORE_DIV_LEN                               (4U)
#define GLB_WIFI_MAC_CORE_DIV_MSK                               (((1U<<GLB_WIFI_MAC_CORE_DIV_LEN)-1)<<GLB_WIFI_MAC_CORE_DIV_POS)
#define GLB_WIFI_MAC_CORE_DIV_UMSK                              (~(((1U<<GLB_WIFI_MAC_CORE_DIV_LEN)-1)<<GLB_WIFI_MAC_CORE_DIV_POS))
#define GLB_WIFI_MAC_WT_DIV                                     GLB_WIFI_MAC_WT_DIV
#define GLB_WIFI_MAC_WT_DIV_POS                                 (4U)
#define GLB_WIFI_MAC_WT_DIV_LEN                                 (4U)
#define GLB_WIFI_MAC_WT_DIV_MSK                                 (((1U<<GLB_WIFI_MAC_WT_DIV_LEN)-1)<<GLB_WIFI_MAC_WT_DIV_POS)
#define GLB_WIFI_MAC_WT_DIV_UMSK                                (~(((1U<<GLB_WIFI_MAC_WT_DIV_LEN)-1)<<GLB_WIFI_MAC_WT_DIV_POS))


#define wait_us(x) bflb_mtimer_delay_ms(x)

//#include "rfc.c"
#include "mac.c"
#include "phy.c"

#include "rxl.c"

void hal_machw_init(void);
void phy_init(void *);
void phy_set_channel(uint8_t band,uint8_t type,uint16_t prim20_freq,uint16_t center1_freq, uint16_t center2_freq,uint8_t index);
void hal_machw_monitor_mode(void);


//void oblfr_rfc_init();
void sysctrl_init(void);
//void bl_init(void);
void rfc_init(uint32_t, uint8_t);

volatile unsigned int wifi_num_irqs = 0;
volatile unsigned int wifi_num_irq2 = 0;

void ffw_wifi_irq() {
    wifi_num_irqs++;
    //LOG_I("WIFI IRQ!\r\n");
}

bool intc_is_pending(int num) {
    return *(volatile uint8_t*)(0x2800000 + num);
}
void intc_clear_pending(int num) {
    *(volatile uint8_t*)(0x2800000 + num) = 0;
}


static int intc_irq_status_get() {
    int ret = *(volatile int*)0x24910000;
    ret |= *(volatile int*)0x24910004;
    return ret;
}

static int intc_irq_index_get() {
    return *(volatile int*)0x24910040;
}

void macss_irq() {
    if (intc_irq_status_get() == 0)
        return;

    int index = intc_irq_index_get();
    LOG_I("mac irq, index: %u\r\n", index);
}

void wifi_fw_app(void *pvParameters) { //Wifi main
    uint32_t aaaaaa[8] = {0};
    uint32_t tmpVal = 0;
    LOG_I("Wifi FW Running...\r\n");

    //Enable the nececarry IRQs
    LOG_I("Starting wifi radio...\r\n");
    phy_powroffset_set((int8_t [4]){0x0, 0x0, 0x0, 0x0});
    bl_tpc_update_power_rate_11b((int8_t [4]){0x14, 0x14, 0x14, 0x12});
    bl_tpc_update_power_rate_11g((int8_t [8]){0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0xe, 0xe});
    bl_tpc_update_power_rate_11n((int8_t [8]){0x12, 0x12, 0x12, 0x12, 0x12, 0x10, 0xe, 0xe});

    //Setup wifi clocks & IRQs
    LOG_I("Setting up WIFI clocks & IRQs...\r\n");
    tmpVal = BL_RD_REG(GLB_BASE, GLB_WIFI_CFG0);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, GLB_WIFI_MAC_CORE_DIV, 0); //0 for 40mhz, 1 for 80mhz
    BL_WR_REG(GLB_BASE, GLB_WIFI_CFG0, tmpVal);
    /*bflb_irq_enable(50); //PDS_WAKEUP_IRQn
    bflb_irq_enable(52); //HBN_OUT1_IRQn
    bflb_irq_enable(WIFI_IRQn); //WIFI_IRQn
    bflb_irq_enable(54); //WIFI_IRQn
    bflb_irq_attach(WIFI_IRQn, ffw_wifi_irq, NULL);*/
    bflb_irq_attach(WIFI_IRQn, ffw_wifi_irq, NULL);
    bflb_irq_enable(WIFI_IRQn); //WIFI_IRQn
    bflb_irq_attach(70, ffw_wifi_irq, NULL);
    bflb_irq_enable(70); //WIFI_IRQn


    *(volatile uint32_t*)0x24b00400 |= 1;
    LOG_I("rfc_init\r\n");
    //fw_rfc_init();
    //oblfr_rfc_init();
    rfc_init(40000000, 1);
    LOG_I("rfc_init finish\r\n");

    //Rest
    ///Should be handled
    sysctrl_init();


    /*LOG_I("mac_init\r\n");
    fw_mac_init();
    LOG_I("mac_init finish\r\n");

    LOG_I("phy_init\r\n");
    fw_phy_init();
    LOG_I("phy_init finish\r\n");*/

    //bl_init();
    hal_machw_init(); ///mac_init
    //////rxl_init
    wifi_rxl_init();

    *(volatile uint32_t*)0x24b00404 = 0x24f037;
    *(volatile uint32_t*)0x24b00400 |= 1;
    *(volatile uint32_t*)0x24b00400 &= 0xfffffffe;
    *(volatile uint32_t*)0x24b00400 = 0x68;
    *(volatile uint32_t*)0x24b00400 |= 1;
    *(volatile uint32_t*)0x24b00400 &= 0xffffffdf;

    //Set frequency
    //////fw_phy_set_ch(2437);
    //Set to monitor mode
    ////fw_mac_config_monitor_mode();
    LOG_I("Starting monitor ...\r\n");
    //fw_mac_config_monitor_mode();
    phy_init(&aaaaaa);
    ////////fw_mac_config_monitor_mode();
    phy_set_channel('\0','\0',0x985,0x985,0,'\0');

    //Set to monitor mode
    hal_machw_monitor_mode();

    //Start, from mm_active
    *(volatile uint32_t*)0x24b00038 = 0x30;

    ////////
    while (1) {
        /*if (intc_is_pending(WIFI_IRQn)) {
            intc_clear_pending(WIFI_IRQn);
             //wifi_num_irq2++;
             macss_irq();
        }*/
        bflb_mtimer_delay_ms(5000);
        LOG_I("IRQs: %u\r\n", wifi_num_irqs);
        /*bflb_mtimer_delay_ms(5000);
        LOG_I("IRQs: %u, %u\r\n", wifi_num_irqs, wifi_num_irq2);*/

        
    }
}
