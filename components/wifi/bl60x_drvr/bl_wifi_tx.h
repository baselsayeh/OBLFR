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
#ifndef __BL_WIFI_TX_H__
#define __BL_WIFI_TX_H__

#define CFG_CHIP_BL808 1

#include <stdint.h>
//#include <utils_list.h>

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define le16_to_cpu(val) (val)
#define cpu_to_le16(val) (val)
#else
#error "Include byteorder header"
#endif

#include <assert.h>

#include "bl_os_hal.h"
#include "bl60x_wifi_driver/errno.h"
#include "bl60x_wifi_driver/lmac_msg.h"
#include "bl60x_wifi_driver/bl_cmds.h"
#include "bl60x_wifi_driver/ipc_shared.h"
#include "bl60x_wifi_driver/reg_ipc_app.h"
#include "bl60x_wifi_driver/nl80211.h"
#include "bl60x_wifi_driver/cfg80211.h"
#include "../prebuilt_libs/wifi/include/bl60x_fw_api.h"

struct bl_send_scanu_para {
    uint16_t *channels;
    uint16_t channel_num;
    struct mac_addr *bssid;
    struct mac_ssid *ssid;
    uint8_t *mac;
    uint8_t scan_mode;
    uint32_t duration_scan;
};

static inline bool is_non_blocking_msg(int id) {
    return ((id == MM_TIM_UPDATE_REQ) || (id == ME_RC_SET_RATE_REQ) ||
            (id == MM_BFMER_ENABLE_REQ) || (id == ME_TRAFFIC_IND_REQ));
}

static inline void *bl_msg_zalloc(ke_msg_id_t const id,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id,
                                    uint16_t const param_len);

int bl_wifi_ipc_send_msg(struct ipc_shared_env_tag *shared, void *msg_buf, uint16_t len);

static int bl_send_msg(void *bl_hw, const void *msg_params,
                         int reqcfm, ke_msg_id_t reqid, void *cfm);

/*int bl_send_reset(struct bl_hw *bl_hw);*/
int bl_send_reset();
int bl_send_monitor_enable(struct mm_monitor_cfm *cfm);
int bl_send_monitor_disable(struct mm_monitor_cfm *cfm);
/*
 *  use_40MHZ:
 *      0: Don't use 40MHZ
 *      1: Use lower band as second band
 *      2: Use higher band as second band
 * */
int bl_send_monitor_channel_set(struct mm_monitor_channel_cfm *cfm, int channel, int use_40Mhz);
int bl_send_version_req(struct mm_version_cfm *cfm);
int bl_send_me_config_req(/*struct bl_hw *bl_hw*/);
int bl_send_me_chan_config_req(/*struct bl_hw *bl_hw*/);
int bl_send_me_rate_config_req(uint8_t sta_idx, uint16_t fixed_rate_cfg);
int bl_send_start(/*struct bl_hw *bl_hw*/);
int bl_send_add_if(const unsigned char *mac,
                     enum nl80211_iftype iftype, bool p2p, struct mm_add_if_cfm *cfm);
int bl_send_remove_if(uint8_t inst_nbr);
int bl_send_scanu_req(struct bl_send_scanu_para *scanu_para);
int bl_send_scanu_raw_send(uint8_t *pkt, int len);
////////int bl_send_sm_connect_req(struct cfg80211_connect_params *sme, struct sm_connect_cfm *cfm);
int bl_send_sm_connect_req(uint8_t vif_idx, uint8_t *ssid, uint8_t *password);
int bl_send_sm_connect_abort_req(struct sm_connect_abort_cfm *cfm);
int bl_send_sm_disconnect_req(/*struct bl_hw *bl_hw*/);
int bl_send_mm_powersaving_req(int mode);
int bl_send_mm_denoise_req(int mode);
int bl_send_apm_start_req(struct apm_start_cfm *cfm, char *ssid, char *password, int channel, uint8_t vif_index, uint8_t hidden_ssid, uint16_t bcn_int);
int bl_send_apm_stop_req(uint8_t vif_idx);
int bl_send_apm_sta_del_req(struct apm_sta_del_cfm *cfm, uint8_t sta_idx, uint8_t vif_idx);
int bl_send_apm_conf_max_sta_req(uint8_t max_sta_supported);
int bl_send_cfg_task_req(uint32_t ops, uint32_t task, uint32_t element, uint32_t type, void *arg1, void *arg2);
int bl_send_channel_set_req(int channel);
void bl_msg_update_channel_cfg(const char *code);
int bl_msg_get_channel_nums();
int bl_get_fixed_channels_is_valid(uint16_t *channels, uint16_t channel_num);
int bl_send_beacon_interval_set(struct mm_set_beacon_int_cfm *cfm, uint16_t beacon_int);

#endif
