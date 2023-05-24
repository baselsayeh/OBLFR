#include <stdint.h>
#include <string.h>
#include <board.h>
#include <bl808.h>
#include <bl808_common.h>
//#include <utils_list.h>

#define DBG_TAG "WIFI_TX"
#include "log.h"
#include <bl_os_hal.h>
#include <bl_os_adapter/bl_os_adapter.h>
#include <bl_os_adapter/bl_os_log.h>

#include "bl_wifi_tx.h"

// #include "bl_os_hal.h"
// #include "bl60x_wifi_driver/lmac_msg.h"
// #include "bl60x_wifi_driver/bl_cmds.h"
// #include "bl60x_wifi_driver/ipc_shared.h"
// #include "bl60x_wifi_driver/reg_ipc_app.h"
// #include "../prebuilt_libs/wifi/include/bl60x_fw_api.h"

static const struct mac_addr mac_addr_bcst = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
static const struct mac_addr mac_addr_zero = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
#define ETH_ALEN 6

//int bl_wifi_ipc_send_msg(struct ipc_host_env_tag *env, void *msg_buf, uint16_t len)
int bl_wifi_ipc_send_msg(struct ipc_shared_env_tag *shared, void *msg_buf, uint16_t len)
{
    int i;
    uint32_t *src, *dst;

    //REG_SW_SET_PROFILING(env->pthis, SW_PROF_IPC_MSGPUSH);

    /*ASSERT_ERR(!env->msga2e_hostid);
    ASSERT_ERR(round_up(len, 4) <= sizeof(env->shared->msg_a2e_buf.msg));*/

    // Copy the message into the IPC MSG buffer
    src = (uint32_t*)((struct bl_cmd *)msg_buf)->a2e_msg;
    dst = (uint32_t*)&(shared->msg_a2e_buf.msg);

    // Copy the message in the IPC queue
    for (i=0; i<len; i+=4)
    {
        *dst++ = *src++;
    }

    //env->msga2e_hostid = msg_buf;

    // Trigger the irq to send the message to EMB
    LOG_D("%s: Trigering ipc_app2emb_trigger_set\r\n", __func__);
    ipc_app2emb_trigger_set(IPC_IRQ_A2E_MSG);

    return 0;
}

static inline void *bl_msg_zalloc(ke_msg_id_t const id,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id,
                                    uint16_t const param_len)
{
    struct lmac_msg *msg;

    msg = (struct lmac_msg *)bl_os_malloc(sizeof(struct lmac_msg) + param_len);
    if (msg == NULL) {
        LOG_E("%s: msg allocation failed\n", __func__);
        return NULL;
    }
    memset(msg, 0, sizeof(struct lmac_msg) + param_len);

    msg->id = id;
    msg->dest_id = dest_id;
    msg->src_id = src_id;
    msg->param_len = param_len;

    return msg->param;
}

//static int bl_send_msg(struct bl_hw *bl_hw, const void *msg_params,
static int bl_send_msg(void *bl_hw, const void *msg_params,
                         int reqcfm, ke_msg_id_t reqid, void *cfm)
{
    struct lmac_msg *msg;
    struct bl_cmd *cmd;
    bool nonblock;
    int ret;

    msg = container_of((void *)msg_params, struct lmac_msg, param);

    /*if (!bl_hw->ipc_env) {
        bl_os_printf("%s: bypassing (restart must have failed)\r\n", __func__);
        bl_os_free(msg);
        RWNX_DBG(RWNX_FN_LEAVE_STR);
        return -EBUSY;
    }*/

    nonblock = is_non_blocking_msg(msg->id);

    cmd = bl_os_malloc(sizeof(struct bl_cmd));
    if (NULL == cmd) {
        bl_os_free(msg);
        LOG_E("%s: failed to allocate mem for cmd, size is %d\r\n", __func__, sizeof(struct bl_cmd));
        return -ENOMEM;
    }
    memset(cmd, 0, sizeof(struct bl_cmd));
    cmd->result  = EINTR;
    cmd->id      = msg->id;
    cmd->reqid   = reqid;
    cmd->a2e_msg = msg;
    cmd->e2a_msg = cfm;
    if (nonblock)
        cmd->flags = RWNX_CMD_FLAG_NONBLOCK;
    if (reqcfm)
        cmd->flags |= RWNX_CMD_FLAG_REQ_CFM;
    //ret = bl_hw->cmd_mgr.queue(&bl_hw->cmd_mgr, cmd);
    ret = bl_wifi_ipc_send_msg(&ipc_shared_env, cmd, sizeof(struct bl_cmd));

    if (!nonblock) {
        bl_os_free(cmd);
    } else {
        ret = cmd->result;
    }

    return ret;
}



//////
int bl_send_reset() {
    void *void_param;

    /* RESET REQ has no parameter */
    void_param = bl_msg_zalloc(MM_RESET_REQ, TASK_MM, DRV_TASK_ID, 0);
    if (!void_param)
        return -ENOMEM;

    return bl_send_msg(NULL, void_param, 1, MM_RESET_CFM, NULL);
}

int bl_send_version_req(struct mm_version_cfm *cfm) {
    int ret;
    void *void_param;

    /* VERSION REQ has no parameter */
    void_param = bl_msg_zalloc(MM_VERSION_REQ, TASK_MM, DRV_TASK_ID, 0);
    if (!void_param) {
        return -ENOMEM;
    }
    ret = bl_send_msg(NULL, void_param, 1, MM_VERSION_CFM, cfm);
    return ret;
}

int bl_send_me_config_req() {
    struct me_config_req *req;
    uint8_t ht_mcs[16] = {0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00}; //Setup in bl_mod_params.c
    int i, ret;

    /* Build the ME_CONFIG_REQ message */
    req = bl_msg_zalloc(ME_CONFIG_REQ, TASK_ME, DRV_TASK_ID,
                                   sizeof(struct me_config_req));
    if (!req) {
        return -ENOMEM;
    }

    /* Set parameters for the ME_CONFIG_REQ message */
    //bl_os_printf("[ME] HT supp %d, VHT supp %d\r\n", 1, 0);

    req->ht_supp = 1;
    req->vht_supp = 0;
    req->ht_cap.ht_capa_info = cpu_to_le16(0x010c); //Setup in bl_mod_params.c

    /*AMPDU MAX Length:  
     * 0x0:8K
     * 0x1:16K
     * 0x2:32K
     * 0x3:64K
     */
    req->ht_cap.a_mpdu_param = 0x3;

    /*for (i = 0; i < sizeof(bl_hw->ht_cap.mcs); i++) {
        req->ht_cap.mcs_rate[i] = ht_mcs[i];
    }*/
    for (i = 0; i < sizeof(ht_mcs); i++) {
        req->ht_cap.mcs_rate[i] = ht_mcs[i];
    }
    req->ht_cap.ht_extended_capa = 0;
    req->ht_cap.tx_beamforming_capa = 0;
    req->ht_cap.asel_capa = 0;

    //TODO talk with firmware guys
#if 0
    req->vht_cap.vht_capa_info = cpu_to_le32(vht_cap->cap);
    req->vht_cap.rx_highest = cpu_to_le16(vht_cap->vht_mcs.rx_highest);
    req->vht_cap.rx_mcs_map = cpu_to_le16(vht_cap->vht_mcs.rx_mcs_map);
    req->vht_cap.tx_highest = cpu_to_le16(vht_cap->vht_mcs.tx_highest);
    req->vht_cap.tx_mcs_map = cpu_to_le16(vht_cap->vht_mcs.tx_mcs_map);
#endif

    req->ps_on = 0; //Setup in bl_mod_params.c, default is 0
    req->tx_lft = 100; //Setup in bl_mod_params.c, default is 100

    /* Send the ME_CONFIG_REQ message to LMAC FW */
    ret = bl_send_msg(NULL, req, 1, ME_CONFIG_CFM, NULL);
    return ret;
}

const uint32_t enabled_channels = 13;
uint16_t centerfreqs_2_4ghz[] = {2412, 2417, 2422, 2427, 2432, 2437, 2442, 2447, 2452, 2457, 2462, 2467, 2472, 2484};

//uint16_t maxpower_2_4ghz[2];
int bl_send_me_chan_config_req() {
    struct me_chan_config_req *req;
    int i;

    /* Build the ME_CHAN_CONFIG_REQ message */
    req = bl_msg_zalloc(ME_CHAN_CONFIG_REQ, TASK_ME, DRV_TASK_ID,
                                            sizeof(struct me_chan_config_req));
    if (!req)
        return -ENOMEM;

    req->chan2G4_cnt = 0;
    for (i = 0; i < enabled_channels; i++) {
        req->chan2G4[i].flags = 0;
        //TODO: Maybe in the future make sure that channel is enabled or disabled
        /*if (channels_default[i].flags & IEEE80211_CHAN_DISABLED)
            req->chan2G4[req->chan2G4_cnt].flags |= SCAN_DISABLED_BIT;
        req->chan2G4[i].flags |= passive_scan_flag(channels_default[i].flags);*/
        req->chan2G4[i].band = NL80211_BAND_2GHZ;
        req->chan2G4[i].freq = centerfreqs_2_4ghz[i];
        req->chan2G4[i].tx_power = 20; //All channels have 20 max power, bl_msg_tx.c

        if (i == SCAN_CHANNEL_2G4)
            break;
    }
    req->chan2G4_cnt = enabled_channels;

    /* Send the ME_CHAN_CONFIG_REQ message to LMAC FW */
    return bl_send_msg(NULL, req, 1, ME_CHAN_CONFIG_CFM, NULL);
}

int bl_send_start() {
    struct mm_start_req *start_req_param;

    /* Build the START REQ message */
    start_req_param = bl_msg_zalloc(MM_START_REQ, TASK_MM, DRV_TASK_ID,
                                      sizeof(struct mm_start_req));
    if (!start_req_param)
        return -ENOMEM;

    memset(&start_req_param->phy_cfg, 0, sizeof(start_req_param->phy_cfg));
    //XXX magic number
    start_req_param->phy_cfg.parameters[0] = 0x1;
    start_req_param->uapsd_timeout = (u32_l)3000; //bl_mod_params.c
    start_req_param->lp_clk_accuracy = (u16_l)20; //bl_mod_params.c

    /* Send the START REQ message to LMAC FW */
    return bl_send_msg(NULL, start_req_param, 1, MM_START_CFM, NULL);
}

int bl_send_scanu_req(struct bl_send_scanu_para *scanu_para) {
    struct scanu_start_req *req;
    int i, index;
    uint8_t chan_flags = 0;
    //const struct ieee80211_channel *chan;

    /* Build the SCANU_START_REQ message */
    req = bl_msg_zalloc(SCANU_START_REQ, TASK_SCANU, DRV_TASK_ID,
                          sizeof(struct scanu_start_req));
    if (!req) {
        return -ENOMEM;
    }

    /* Set parameters */
    // Always use idx 0, because vif_idx in vif_entry could be 0, leading to probe_rep tx fail
    req->vif_idx = 0;
    /*if (0 == scanu_para->channel_num) {
        req->chan_cnt = channel_num_default;
    } else {
        req->chan_cnt = scanu_para->channel_num;
    }*/
    //Scan all channels
    req->chan_cnt = enabled_channels;

    //req->ssid_cnt = 1;
    req->ssid_cnt = 0;
    /*if (scanu_para->ssid != NULL && scanu_para->ssid->length) {
        req->ssid[0].length = scanu_para->ssid->length;
        memcpy(req->ssid[0].array, scanu_para->ssid->array, req->ssid[0].length);
    } else {*/
        req->ssid[0].length = 0;
        //if specfied ssid, ignore user setting passive mode
        //if (req->ssid_cnt == 0 || scanu_para->scan_mode == SCAN_PASSIVE) 
        //{
            chan_flags |= SCAN_PASSIVE_BIT;
        //}
    //}
    /*memcpy((uint8_t *)&(req->bssid), (uint8_t *)scanu_para->bssid, ETH_ALEN);
    memcpy(&(req->mac), scanu_para->mac, ETH_ALEN);*/
    scanu_para->mac[0] = 0xDE;
    scanu_para->mac[1] = 0xAD;
    scanu_para->mac[2] = 0xBE;
    scanu_para->mac[3] = 0xEF;
    scanu_para->mac[4] = 0xF0;
    scanu_para->mac[5] = 0x0D;
    req->no_cck = true; //FIXME params? talk with firmware guys

#if 0
    for (i = 0; i < req->ssid_cnt; i++) {
        int j;
        for (j = 0; j < param->ssids[i].ssid_len; j++)
            req->ssid[i].array[j] = param->ssids[i].ssid[j];
        req->ssid[i].length = param->ssids[i].ssid_len;
    }
#endif

    //XXX custom ie can be added
    req->add_ie_len = 0;
    req->add_ies = 0;

    for (i = 0; i < req->chan_cnt; i++) {
        //index = (channel_num_default == req->chan_cnt) ? i : (scanu_para->channels[i] - 1);
        index = i;
        //chan = &(channels_default[index]);

        /*req->chan[i].band = chan->band;
        req->chan[i].freq = chan->center_freq;
        req->chan[i].flags = chan_flags | passive_scan_flag(chan->flags);
        req->chan[i].tx_power = chan->max_reg_power;*/
        req->chan[i].band = NL80211_BAND_2GHZ;
        req->chan[i].freq = centerfreqs_2_4ghz[i];
        req->chan[i].tx_power = 20;
    }

    /* Send the SCANU_START_REQ message to LMAC FW */
    return bl_send_msg(NULL, req, 0, 0, NULL);
}
//////
#include "hex_print.h"
int bl_send_add_if(const unsigned char *mac,
                     enum nl80211_iftype iftype, bool p2p, struct mm_add_if_cfm *cfm)
{
    struct mm_add_if_req *add_if_req_param;

    /* Build the ADD_IF_REQ message */
    add_if_req_param = bl_msg_zalloc(MM_ADD_IF_REQ, TASK_MM, DRV_TASK_ID,
                                       sizeof(struct mm_add_if_req));
    if (!add_if_req_param)
        return -ENOMEM;

    /* Set parameters for the ADD_IF_REQ message */
    memcpy(&(add_if_req_param->addr.array[0]), mac, ETH_ALEN);
    switch (iftype) {
    case NL80211_IFTYPE_P2P_CLIENT:
        add_if_req_param->p2p = true;
        __attribute__((fallthrough));
        // no break
    case NL80211_IFTYPE_STATION:
        add_if_req_param->type = MM_STA;
        break;

    case NL80211_IFTYPE_ADHOC:
        add_if_req_param->type = MM_IBSS;
        break;

    case NL80211_IFTYPE_P2P_GO:
        add_if_req_param->p2p = true;
        __attribute__((fallthrough));
        // no break
    case NL80211_IFTYPE_AP:
        add_if_req_param->type = MM_AP;
        break;
    case NL80211_IFTYPE_MESH_POINT:
        add_if_req_param->type = MM_MESH_POINT;
        break;
    case NL80211_IFTYPE_AP_VLAN:
        return -1;
    default:
        add_if_req_param->type = MM_STA;
        break;
    }

    print_hx(add_if_req_param, sizeof(*add_if_req_param));
    /* Send the ADD_IF_REQ message to LMAC FW */
    //return bl_send_msg(NULL, add_if_req_param, 1, MM_ADD_IF_CFM, cfm);
    return bl_send_msg(NULL, add_if_req_param, 1, MM_ADD_IF_CFM, cfm);
}

static struct sm_connect_cfm cfm22;
//int bl_send_sm_connect_req(struct bl_hw *bl_hw, struct cfg80211_connect_params *sme, struct sm_connect_cfm *cfm)
int bl_send_sm_connect_req(uint8_t vif_idx, uint8_t *ssid, uint8_t *password)
{
    struct sm_connect_req *req;
    int i;
    //u32_l flags = sme->flags;
    u32_l flags = 0; //TODO: CHANGE

    //MY
    struct sm_connect_cfm *cfm;
    cfm = &cfm22;
    memset(cfm, 0x00, sizeof(cfm));
    //

    /* Build the SM_CONNECT_REQ message */
    req = bl_msg_zalloc(SM_CONNECT_REQ, TASK_SM, DRV_TASK_ID,
                                   sizeof(struct sm_connect_req));
    if (!req)
        return -ENOMEM;

    req->ctrl_port_ethertype = 0x8e88;

    /*if (sme->bssid && !MAC_ADDR_CMP(sme->bssid, mac_addr_bcst.array) && !MAC_ADDR_CMP(sme->bssid, mac_addr_zero.array)) {
        for (i=0;i<ETH_ALEN;i++)
            req->bssid.array[i] = sme->bssid[i];
    }
    else*/
        req->bssid = mac_addr_bcst;

    req->vif_idx = vif_idx;
    /*if (sme->channel.center_freq) {
        req->chan.band = sme->channel.band;
        req->chan.freq = sme->channel.center_freq;
        req->chan.flags = passive_scan_flag(sme->channel.flags);
    } else {*/
        //Always scan
        req->chan.freq = (u16_l)-1;
    //}
    for (i = 0; i < strlen(ssid); i++)
        req->ssid.array[i] = ssid[i];
    req->ssid.length = strlen(ssid);
    req->flags = flags;
/*#if 0 // useless
    if (WARN_ON(sme->ie_len > sizeof(req->ie_buf)))
        return -EINVAL;
    if (sme->ie_len)
        memcpy(req->ie_buf, sme->ie, sme->ie_len);
    req->ie_len = sme->ie_len;
#endif*/
    req->listen_interval = 1; //from bl_mod_params.c
    req->dont_wait_bcmc = false; //from bl_mod_params.c

    /* Set auth_type */
    //if (sme->auth_type == NL80211_AUTHTYPE_AUTOMATIC)
        req->auth_type = NL80211_AUTHTYPE_OPEN_SYSTEM;
    /*else
        req->auth_type = sme->auth_type;*/

    /* Set UAPSD queues */
    req->uapsd_queues = 0; //from bl_mod_params.c
    req->is_supplicant_enabled = 1;
    /*if (sme->key_len) {
        memcpy(req->phrase, sme->key, sme->key_len);
    }
    if (sme->pmk_len) {
        memcpy(req->phrase_pmk, sme->pmk, sme->pmk_len);
    }*/

    /* Send the SM_CONNECT_REQ message to LMAC FW */
    //return bl_send_msg(bl_hw, req, 1, SM_CONNECT_CFM, cfm);
    return bl_send_msg(NULL, req, 1, SM_CONNECT_CFM, cfm);
}
