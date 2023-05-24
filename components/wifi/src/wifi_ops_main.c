#include <stdbool.h>
#include <string.h>
#include <sys/queue.h>
#include <board.h>
#include <bl808.h>
#include <bl808_common.h>
#include <bl808_glb.h>
#include <ipc_reg.h>

//FreeRTOS
#ifndef CONFIG_FREERTOS
#error "FreeRTOS MUST be enabled"
#endif
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "oblfr_wifi.h"
#include "../bl60x_drvr/bl_wifi_tx.h"
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

void wifi_ops_thread(void *pvParameters) {
    int ret;
    struct mm_add_if_cfm add_if_cfm;
    const unsigned char mac[] = {0x18, 0xb9, 0x05, 0x88, 0x88, 0x88};

    LOG_I("%s started\r\n", __func__);
    bl_os_sleep(10); //Sleep for 10 sec

    //Send fw reset
    LOG_I("Sending fw reset...\r\n");
    ret = bl_send_reset();
    bl_os_msleep(500); //only 5msec in bl_main

    //Get Version
    LOG_I("Getting fw version...\r\n");
    struct mm_version_cfm version_cfm = {};
    ret = bl_send_version_req(&version_cfm);

    //Send parameters to firmware
    LOG_I("Sending fw params & channel config...\r\n");
    ret = bl_send_me_config_req();
    bl_os_msleep(500);
    ret = bl_send_me_chan_config_req();

    //Accually start the PHY after is has been configured
    LOG_I("Starting PHY...\r\n");
    ret = bl_send_start();
    bl_os_msleep(500);

    //Request a scan search
    /*LOG_I("Sending a scan request...\r\n");
    bl_send_scanu_req(NULL);*/

    LOG_I("Adding IF...\r\n");
    bl_send_add_if(mac, NL80211_IFTYPE_STATION, false, &add_if_cfm);
    LOG_I("IF Added, CFM:\r\n");
    print_hx(&add_if_cfm, sizeof(add_if_cfm));

    /*LOG_I("Connecting to network...\r\n");
    bl_send_sm_connect_req(1, "LEDE_4G", NULL);*/

    for (;;) {}
}
