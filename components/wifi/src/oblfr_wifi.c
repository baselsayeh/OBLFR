#include <stdbool.h>
#include <string.h>
#include <sys/queue.h>
#include <board.h>
#include <bl808.h>
#include <bl808_common.h>
#include <bl808_glb.h>
#include <ipc_reg.h>
#include <bflb_clock.h>
#include <bflb_gpio.h>

//FreeRTOS
#ifndef CONFIG_FREERTOS
#error "FreeRTOS MUST be enabled"
#endif
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "oblfr_wifi.h"
#include "wifi_ops_main.h"
#include "bl60x_fw_api.h"
//#include "bl_phy_api.h"
#define DBG_TAG "WIFI"
#include "log.h"

#ifndef CONFIG_CHIP_BL808
#error "This component is only for BL808"
#endif

#include <bl_os_hal.h>
#include <bl_os_adapter/bl_os_adapter.h>
#include <bl_os_adapter/bl_os_log.h>
extern bl_ops_funcs_t g_bl_ops_funcs;
extern void bl_os_log_write(uint32_t level, const char *tag, const char *file, int line, const char *format, ...);

void mac_irq(void);
void bl_irq_handler(void);

oblfr_err_t oblfr_wifi_init()
{
    int i;
    uint32_t tmpVal = 0;

#ifdef CONFIG_FREERTOS
    //oblfr_mbox_signals_lock = xSemaphoreCreateMutex();
#endif

    /* setup the IPC Interupt */
    /*bflb_irq_attach(IPC_M0_IRQn, IPC_M0_IRQHandler, NULL);
    BL_WR_REG(IPC0_BASE, IPC_CPU0_IPC_IUSR, 0xffffffff);
    bflb_irq_enable(IPC_M0_IRQn);*/

    /* register our Interupt Handlers to forward over */

    /*for (i = 0; i < sizeof(ipc_irqs) / sizeof(ipc_irqs[0]); i++) {
        if (ipc_irqs[i].irq != 0) {
            LOG_I("Forwarding Interupt %s (%d) to D0 (%p)\r\n", ipc_irqs[i].name, ipc_irqs[i].irq, ipc_irqs[i].handler);
            bflb_irq_attach(ipc_irqs[i].irq, ipc_irqs[i].handler, NULL);
            bflb_irq_enable(ipc_irqs[i].irq);
        }
    }*/

    //Just init the wifi
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
    //IRQs
    /*blfb_irq_register(WIFI_IRQn, mac_irq);
    blfb_irq_register(WIFI_IPC_PUBLIC_IRQn, bl_irq_handler);*/
    bflb_irq_attach(WIFI_IRQn, mac_irq, NULL);
    bflb_irq_attach(WIFI_IPC_PUBLIC_IRQn, bl_irq_handler, NULL);
    bflb_irq_enable(WIFI_IRQn);
    bflb_irq_enable(WIFI_IPC_PUBLIC_IRQn);

    //Start the main task
    LOG_I("Creating WIFI FW task...\r\n");
    xTaskCreate(wifi_main, "wifi_fw", 1536, NULL, 3, NULL);
    //xTaskCreate(wifi_main, "wifi_fw", 2048, NULL, 3, NULL);
    xTaskCreate(wifi_ops_thread, "wifi_ops_thread", 2048, NULL, 2, NULL);

    return OBLFR_OK;
}

oblfr_err_t oblfr_wifi_dump() 
{
    return OBLFR_OK;
}
