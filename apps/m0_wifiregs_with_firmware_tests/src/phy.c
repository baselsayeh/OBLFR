/* PHY */

//*(volatile uint32_t*)0x

void fw_phy_agc_config() {
    uint32_t temp;

    *(volatile uint32_t*)0x24c0b390 &= 0xfffefbff;
    *(volatile uint32_t*)0x24c0b3a4 &= 0xffff0000;

    temp = *(volatile uint32_t*)0x24c0b394;
    *(volatile uint32_t*)0x24c0b394 = (temp & 0xff00ffff) | 0xf80000;

    temp = *(volatile uint32_t*)0x24c0b398;
    *(volatile uint32_t*)0x24c0b398 = (temp & 0xffff00ff) | 0x9e00;

    temp = *(volatile uint32_t*)0x24c0b3c4;
    *(volatile uint32_t*)0x24c0b3c4 = (temp & 0xffffff00) | 0xce;

    temp = *(volatile uint32_t*)0x24c0b364;
    *(volatile uint32_t*)0x24c0b364 = (temp & 0xe0c0c0c0) | 0x83c3839;

    temp = *(volatile uint32_t*)0x24c0b368;
    *(volatile uint32_t*)0x24c0b368 = (temp & 0xffc00c00) | 0x70070;

    temp = *(volatile uint32_t*)0x24c0b36c;
    *(volatile uint32_t*)0x24c0b36c = (temp & 0xf800f800) | 0x7280510;

    temp = *(volatile uint32_t*)0x24c0b370;
    *(volatile uint32_t*)0x24c0b370 = (temp & 0xff80ffff) | 0x580000;

    temp = *(volatile uint32_t*)0x24c0b3c0;
    *(volatile uint32_t*)0x24c0b3c0 = (temp & 0xffffff) | 0x18000000;

    temp = *(volatile uint32_t*)0x24c0b380;
    *(volatile uint32_t*)0x24c0b380 = (temp & 0x3ff) | 0x77f8400;

    temp = *(volatile uint32_t*)0x24c0b384;
    *(volatile uint32_t*)0x24c0b384 = (temp & 0x3ff) | 0xe7750800;

    temp = *(volatile uint32_t*)0x24c0b388;
    *(volatile uint32_t*)0x24c0b388 = (temp & 0x3ff) | 0x3d7a9400;

    temp = *(volatile uint32_t*)0x24c0b38c;
    *(volatile uint32_t*)0x24c0b38c = (temp & 0x23ff) | 0x64038800;

    temp = *(volatile uint32_t*)0x24c0c830;
    *(volatile uint32_t*)0x24c0c830 = (temp & 0x23ff) | 0xfc1d9400;

    temp = *(volatile uint32_t*)0x24c0c814;
    *(volatile uint32_t*)0x24c0c814 = (temp & 0xffffffc0) | 8;

    temp = *(volatile uint32_t*)0x24c0c040;
    *(volatile uint32_t*)0x24c0c040 = (temp & 0xfe007fff) | 0xc18000;

    temp = *(volatile uint32_t*)0x24c0c044;
    *(volatile uint32_t*)0x24c0c044 = (temp & 0xffff0000) | 0x800;

    //phy_config_rxgain

    temp = *(volatile uint32_t*)0x24c0b3a0;
    *(volatile uint32_t*)0x24c0b3a0 = (temp & 0xffffff00) | 0x9e;

    temp = *(volatile uint32_t*)0x24c0b3c0;
    *(volatile uint32_t*)0x24c0b3c0 = (temp & 0xffff0000) | 0xa3b0;

    temp = *(volatile uint32_t*)0x24c0c838;
    *(volatile uint32_t*)0x24c0c838 = (temp & 0x7ff80000) | 0x80000040;

    temp = *(volatile uint32_t*)0x24c0c83c;
    *(volatile uint32_t*)0x24c0c83c = (temp & 0x7ff00000) | 0x80000040;

    temp = *(volatile uint32_t*)0x24c0c840;
    *(volatile uint32_t*)0x24c0c840 = (temp & 0x7fc00000) | 0x80000040;

    temp = *(volatile uint32_t*)0x24c0c82c;
    *(volatile uint32_t*)0x24c0c82c = (temp & 0xff007fff) | 0x200000;

    temp = *(volatile uint32_t*)0x24c0c850;
    *(volatile uint32_t*)0x24c0c850 = (temp & 0xffff) | 0x83c0000;

    temp = *(volatile uint32_t*)0x24c0b304;
    *(volatile uint32_t*)0x24c0b304 = (temp & 0xffff80ff) | 0x4200;

    temp = *(volatile uint32_t*)0x24c0c85c;
    *(volatile uint32_t*)0x24c0c85c = (temp & 0xff00ffff) | 0x300000;
}

#include "mac.h"
void fw_phy_agc_download() {
    *(volatile uint32_t*)0x24c0b390 |= (~0xffffefff);
    *(volatile uint32_t*)0x24c00874 |= (~0xdfffffff);

    volatile uint32_t* agcram = (volatile uint32_t*)0x24c0a000;
    for (unsigned int i=0; i<(sizeof(agcmem)/4); i++)
        agcram[i] = agcmem[i];

    *(volatile uint32_t*)0x24c0b390 &= 0xffffefff;
    *(volatile uint32_t*)0x24c00874 &= 0xdfffffff;
}

void fw_phy_ldcp_download() {
    volatile uint32_t* ldcpram = (volatile uint32_t*)0x24c09000;
    for (unsigned int i=0; i<(sizeof(ldpcmem)/4); i++)
        ldcpram[i] = ldpcmem[i];
}

void fw_phy_rfc_set_freq(uint32_t freq) { //From phy_set_channel & rfc_config_channel
//
    uint32_t temp;

    //in rfc_config_channel
    *(volatile uint32_t*)0x20001228 |= 8;
    *(volatile uint32_t*)0x2000102c |= 0x241;
    //#error "TODO:"
    //////*(volatile uint32_t*)20001264 = channel_freq & 0xfff | _DAT_20001264 & 0xfffff000;
    //Set the actual frequency
    temp = *(volatile uint32_t*)0x20001264;
    *(volatile uint32_t*)0x20001264 = (freq & 0xfff) | (temp & 0xfffff000);
    //

    temp = *(volatile uint32_t*)0x20001268;
    *(volatile uint32_t*)0x20001268 = (temp & 0xfffdffff) | 0x20000;
    wait_us(1);
    *(volatile uint32_t*)0x20001268 &= 0xfffdffff;
    wait_us(1);
    *(volatile uint32_t*)0x20001004 |= 2;
    wait_us(1);
    temp = *(volatile uint32_t*)0x2000126c;
    *(volatile uint32_t*)0x2000126c = (temp & 0xfffffff8) | 1;
    wait_us(1);
    *(volatile uint32_t*)0x2000126c |= 8;
    wait_us(1);
    temp = *(volatile uint32_t*)0x2000126c;
    *(volatile uint32_t*)0x2000126c = (temp & 0xfffffff8) | 2;
    wait_us(100);
    *(volatile uint32_t*)0x2000126c &= 0xfffffff7;
    wait_us(1);
    *(volatile uint32_t*)0x20001228 &= 0xfffffff7;
}
void fw_phy_set_ch(uint32_t freq) { //phy_set_channel
    //Set in phy_set_channel
    *(volatile uint32_t*)0x24c00888 = 0x0;

    fw_phy_rfc_set_freq(freq);
}

void fw_phy_init() { //From phy_init
    uint32_t temp;

    // MDM conf 20Mhz
    *(volatile uint32_t*)0x24c00800 = 0;
    //mdm_reset() skipped

    *(volatile uint32_t*)0x24c00888 = 0x0;
    // MDM rxchan & rxnssmax
    *(volatile uint32_t*)0x24c00820 = 0x130d;

    temp = *(volatile uint32_t*)0x24c03024;
    *(volatile uint32_t*)0x24c03024 = (temp & 0xffc0ffff) | 0x2d0000;

    *(volatile uint32_t*)0x24c0089c = 0xffffffff;
    *(volatile uint32_t*)0x24c00824 = 0x10d;

    *(volatile uint32_t*)0x24c00834 |= 1;

    *(volatile uint32_t*)0x24c00818 &= 0xfffbffff;

    temp = *(volatile uint32_t*)0x24c00830;
    *(volatile uint32_t*)0x24c00830 = (temp & 0xffff0000) | 0x1b0f;

    *(volatile uint32_t*)0x24c0083c = 0x4920492;

    temp = *(volatile uint32_t*)0x24c00874;
    *(volatile uint32_t*)0x24c00874 = (temp & 0xf7ffffff) | 0x8000000;

    temp = *(volatile uint32_t*)0x24c0b500;
    *(volatile uint32_t*)0x24c0b500 = (temp & 0xffffcfff) | 0x2000;

    //No idea what this is
    temp = *(volatile uint32_t*)0x24c0c80c;
    *(volatile uint32_t*)0x24c0c80c = (temp & 0xffffff) | 0x97000000;
    //

    *(volatile uint32_t*)0x24c0b004 = 1;

    temp = *(volatile uint32_t*)0x24c0b390;
    *(volatile uint32_t*)0x24c0b390 = (temp & 0xfffffffc) | 1;

    *(volatile uint32_t*)0x24c0b3bc = 4000000;

    //temp = *(volatile uint32_t*)0x24c0b414;
    *(volatile uint32_t*)0x24c0b414 |= 0x100;

    fw_phy_agc_config();
    fw_phy_agc_download();

    *(volatile uint32_t*)0x24c00838 = 0xb4;
    *(volatile uint32_t*)0x24c0088c = 0x1c13;
    *(volatile uint32_t*)0x24c00898 = 0x2d00438;
    *(volatile uint32_t*)0x24c00858 &= 0xffffff00;
    *(volatile uint32_t*)0x24c0081c = 0xf0f;
    temp = *(volatile uint32_t*)0x24c00834;
    *(volatile uint32_t*)0x24c00834 = (temp & 0xffffff) | 0x6000000;
    *(volatile uint32_t*)0x24c00818 = 0x1880c06;
    *(volatile uint32_t*)0x24c00860 = 0x7f03;
    *(volatile uint32_t*)0x24c0b340 = 0;
    *(volatile uint32_t*)0x24c0b344 = 0;
    *(volatile uint32_t*)0x24c0b348 = 0;
    *(volatile uint32_t*)0x24c00824 &= 0xfcffffff;
    fw_phy_ldcp_download();
    temp = *(volatile uint32_t*)0x2000126c;
    *(volatile uint32_t*)0x2000126c = (temp & 0xc00fffff) | 0x5000000;
    temp = *(volatile uint32_t*)0x24c0c018;
    *(volatile uint32_t*)0x24c0c018 = (temp & 0xfffffc00) | 0x50;

    //fw_phy_set_ch(2437);
}
