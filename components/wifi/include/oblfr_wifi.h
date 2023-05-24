#ifndef __OBLFR_WIFI_H__
#define __OBLFR_WIFI_H__

#include <stdint.h>
#include <stdbool.h>

#include "oblfr_common.h"
#include "sdkconfig.h"

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

oblfr_err_t oblfr_wifi_init(void);
oblfr_err_t oblfr_wifi_dump();

#endif
