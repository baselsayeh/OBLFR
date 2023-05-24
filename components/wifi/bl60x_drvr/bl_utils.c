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

#include <string.h>

#include <assert.h>
/*#include <lwip/api.h>
#include <lwip/opt.h>
#include <lwip/def.h>
#include <lwip/mem.h>
#include <lwip/pbuf.h>
#include <lwip/netif.h>
#include <wifi_mgmr_ext.h>
#include <wifi_pkt_hooks.h>*/

/*#include "ipc_shared.h"
#include "ipc_host.h"*/
#include "bl_utils.h"
#include "bl_wifi_tx.h"
/*#include "bl_rx.h"
#include "bl_tx.h"
#include "bl_cmds.h"*/

#ifdef CFG_NETBUS_WIFI_ENABLE
#include <netbus_mgmr.h>
#include <netbus_utils.h>
#include <utils_log.h>
#endif

#define DBG_TAG "BL606_UTILS"
#include <log.h>

#include "bl_os_hal.h"

#undef os_printf
#define os_printf(...) do {} while(0)

//extern struct bl_hw wifi_hw;

#define MAC_FMT "%02X%02X%02X%02X%02X%02X"
#define MAC_LIST(arr) (arr)[0], (arr)[1], (arr)[2], (arr)[3], (arr)[4], (arr)[5]

int tcpip_stack_input(void *swdesc, uint8_t status, void *hwhdr, unsigned int msdu_offset, struct wifi_pkt *pkt, uint8_t extra_status)
{
    LOG_I("tcpip_stack_input Called!\r\n");
    return 0;
}

void bl_utils_dump(void)
{
}

////
int bl_supplicant_init(void) {
    LOG_I("bl_supplicant_init Called!\r\n");
}
void bl_main_event_handle(int param, void *aaa) {
    LOG_I("bl_main_event_handle Called!\r\n");
}
void bl_rx_e2a_handler(void *arg) {
    struct ipc_e2a_msg *msg = (struct ipc_e2a_msg*)arg;
    uint8_t *aa = (uint8_t *)&msg->param;

    LOG_I("%s Called, param %u len %u!\r\n",  __func__, msg->id, msg->param_len);

    for (int i=0; i<msg->param_len; i+=16)
                LOG_I("%08x: %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x\r\n",
                    i, \
                    aa[i+0], aa[i+1], aa[i+2], aa[i+3],
                    aa[i+4], aa[i+5], aa[i+6], aa[i+7],
                    aa[i+8], aa[i+9], aa[i+10], aa[i+11],
                    aa[i+12], aa[i+13], aa[i+14], aa[i+15]);

    switch (msg->id) {
        case SCANU_START_REQ:
        case SCANU_START_CFM:
            LOG_I("Scanning started...\r\n");
            break;

        case SCANU_RESULT_IND:
            LOG_I("Scan results: %u\r\n", msg->param_len);
            break;

        default:
            LOG_I("%s: Unknown param %u\r\n", __func__, msg->id);
    }
}
void ipc_host_disable_irq_e2a(void) {
    ipc_emb2app_unmask_clear(IPC_IRQ_E2A_ALL);
}
