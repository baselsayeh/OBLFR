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

//For cache invalidation
#include <bl808_glb.h>

//Free RTOS
#include "FreeRTOSConfig.h"
#include <FreeRTOS.h>
#include <task.h>

#define DBG_TAG "MAIN"
#include <log.h>

#include "wifi.c"

void mailbox_wait_dump(void *pvParameters)
{
    unsigned char buf[512];
    while (1)
    {
        memset(buf, 0x00, sizeof(buf));
        vTaskList((char *)&buf);
        LOG_I("\r\nTask  State   Prio    Stack    Num\r\n%s", buf);
        //LOG_I("**********************************\r\n");
        //bflb_mtimer_delay_ms(5000);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

int main(void)
{
    ///Disable caches
    L1C_DCache_Clean_Invalid_All();
    L1C_DCache_Clean_All();
    L1C_DCache_Disable();

    L1C_ICache_Invalid_All();
    L1C_ICache_Disable();
    //
    board_init();

    LOG_I("Hello World!\r\n");
    bflb_mtimer_delay_ms(5000);

    LOG_I("Running...\r\n");

    xTaskCreate(wifi_fw_app, "wifi_fw", CONFIG_COMPONENT_SYSTEM_MAIN_TASK_STACK_SIZE, NULL, CONFIG_COMPONENT_SYSTEM_MAIN_TASK_PRIORITY, NULL);
    if (xTaskCreate(mailbox_wait_dump, "freertos_dump", CONFIG_COMPONENT_SYSTEM_MAIN_TASK_STACK_SIZE, NULL, CONFIG_COMPONENT_SYSTEM_MAIN_TASK_PRIORITY, NULL) != pdPASS) {
        LOG_E("Failed to create mailbox_dump task");
        for (;;) {}
    }
    vTaskStartScheduler();
}
