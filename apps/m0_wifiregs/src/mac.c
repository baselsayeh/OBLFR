/* MAC */
void fw_mac_setfreq() {
    //blmac_mac_core_clk_freq_setf, hardcoded to 40 in fw
    uint32_t temp = *(volatile uint32_t*)0x24b000e4;
    //*(volatile uint32_t*)0x24b000e4 = (temp & ~0xff) | (v & 0xff);
    *(volatile uint32_t*)0x24b000e4 = (temp & ~0xff) | (40 & 0xff);

    //blmac_tx_rf_delay_in_mac_clk_setf
    temp = *(volatile uint32_t*)0x24b000e4;
    *(volatile uint32_t*)0x24b000e4 = (temp & 0xfffc00ff) | ((0 & 0x3ff) << 8);

    //blmac_tx_chain_delay_in_mac_clk_setf
    temp = *(volatile uint32_t*)0x24b000e4;
    *(volatile uint32_t*)0x24b000e4 = (temp & 0xf003ffff) | ((136 & 0x3ff) << 18);

    //0x24b000e8 skipped

    //blmac_rx_rf_delay_in_mac_clk_setf
    temp = *(volatile uint32_t*)0x24b000ec;
    *(volatile uint32_t*)0x24b000ec = (temp & 0xc00fffff) | ((39 & 0x3ff) << 20);

    //blmac_mac_proc_delay_in_mac_clk_setf
    temp = *(volatile uint32_t*)0x24b000ec;
    *(volatile uint32_t*)0x24b000ec = (temp & 0xc00fffff) | ((180 & 0x3ff) << 20);

    //blmac_wt_2_crypt_clk_ratio_setf
    temp = *(volatile uint32_t*)0x24b000f0;
    *(volatile uint32_t*)0x24b000f0 = (temp & ~0x3) | (3 & 0x3);

    //24b000f4 skipped
    //24b000f8 skipped
    //24b00104 skipped *2
}
uint32_t fw_blmac_soft_reset_getf() {
    return (*(volatile uint32_t*)0x24b08050) & 1;
}

void fw_mac_stop() {
    *(volatile uint32_t *)0x24b08050 = 1;

    while (fw_blmac_soft_reset_getf() != 0);
}

void fw_mac_init() { //hal_machw_init
    uint32_t temp;

    //From hal_machw_init
    fw_mac_stop();

    // coex stuff
    *(volatile uint32_t*)0x24b00404 = 0x24f637;
    // toggling coex enable to reset?
    *(volatile uint32_t*)0x24b00400 |= 1; //*(volatile uint32_t*)0x24b00400 = 1;
    *(volatile uint32_t*)0x24b00400 &= 0xfffffffe; //*(volatile uint32_t*)0x24b00400 = 0;
    *(volatile uint32_t*)0x24b00400 = 0x69; //*(volatile uint32_t*)0x24b00400 = 0x68;
    // disable coexAutoPTIAdj
    *(volatile uint32_t*)0x24b00400 &= ~0x20;
    // sysctrl92 ptr config
    *(volatile uint32_t*)0x24920004 = 0x5010001f;
    //???, TODO: Not found in bl60x,wifimon
    *(volatile uint32_t*)0x24b00410 = 1;
    *(volatile uint32_t*)0x24920028 = 10;

    fw_mac_setfreq();
    //blmac_gen_int_enable_set
    *(volatile uint32_t*)0x24b08074 = 0x8373f14c;

    //blmac_rate_controller_mpif_setf
    temp = *(volatile uint32_t*)0x24b0004c;
    *(volatile uint32_t*)0x24b0004c = (temp & 0xfffff7ff) | ((0 & 0x1) << 11);

    //continue in mac_init()
    //24b000d8
    // encryption ram config
    *(volatile uint32_t*)0x24b000d8 = 0x20c08;
    // tx rx int enable
    *(volatile uint32_t*)0x24b08080 = 0x800a07c0;
    // mac cntrl settings: activeClkGating, disable(ACK|CTS|BAR)Resp, rxRIFSEn
    *(volatile uint32_t*)0x24b0004c |= 0x40007c0;

    //blmac_rx_flow_cntrl_en_setf
    temp = *(volatile uint32_t*)0x24b00054;
    *(volatile uint32_t*)0x24b00054 = (temp & 0xfffeffff) | ((1 & 0x1) << 16);

    //blmac_rx_cntrl_set
    *(volatile uint32_t*)0x24b00060 = 0x7fffffde;

    //Continue
    // rx trigger timer
    *(volatile uint32_t*)0x24b00114 = 0x5010a;
    // beacon control 1
    *(volatile uint32_t*)0x24b00064 = 0xff900064;

    //blmac_max_rx_length_set
    *(volatile uint32_t*)0x24b00150 = 0x1000 & 0xfffff;

    //Continue
    // edca control
    *(volatile uint32_t*)0x24b00224 = 0;
    // max power level (ofdm/dsss = 32)
    *(volatile uint32_t*)0x24b000a0 = 0x2020;

    //blmac_mib_table_reset_setf
    temp = *(volatile uint32_t*)0x24b0004c;
    *(volatile uint32_t*)0x24b0004c = (temp & 0xffffefff) | ((1 & 0x1) << 12);
    //blmac_key_sto_ram_reset_setf
    temp = *(volatile uint32_t*)0x24b0004c;
    *(volatile uint32_t*)0x24b0004c = (temp & 0xffffdfff) | ((1 & 0x1) << 13);

    //Continue
    // debug port sel
    *(volatile uint32_t*)0x24b00510 = 0x1c25;
    //blmac_dyn_bw_en_setf
    temp = *(volatile uint32_t*)0x24b00310;
    *(volatile uint32_t*)0x24b00310 = (temp & 0xffffff7f) | ((1 & 0x1) << 7);

    //blmac_tsf_mgt_disable_setf
    temp = *(volatile uint32_t*)0x24b0004c;
    *(volatile uint32_t*)0x24b0004c = (temp & 0xfdffffff) | ((1 & 0x1) << 25);
}






void fw_mac_config_monitor_mode(void) { //hal_machw_monitor_mode
    uint32_t temp;

    //Disable all VIFs
    *(volatile uint32_t*)0x24b08074 &= 0xfffffffe;
    *(volatile uint32_t*)0x24b08074 &= 0xfffffffd;

    *(volatile uint32_t*)0x24b0004c |= 0x700;
    *(volatile uint32_t*)0x24b00060 = 0x7fffffde; //

    temp = *(volatile uint32_t*)0x24b0004c;
    *(volatile uint32_t*)0x24b0004c = (temp & 0xfffe3fff) | 0xc000;
    temp = *(volatile uint32_t*)0x24b0004c;
    *(volatile uint32_t*)0x24b0004c = (temp & 0xffffdfff) | 0x2000;
}

/* // MAC */