void rf_pri_init(uint8_t reset,uint8_t chipv) {
    uint32_t temp;

    temp = *(volatile uint32_t*)0x20000830;
    *(volatile uint32_t*)0x20000830 = (temp & 0xfffff9ff) | 0x1ff;
    //Printf ...

    temp = *(volatile uint32_t*)0x2000f820;
    *(volatile uint32_t*)0x2000f820 = (temp & 0xff0fffff) | 0x300000;
    temp = *(volatile uint32_t*)0x2000f030;
    *(volatile uint32_t*)0x2000f030 = (temp & 0xf0ffffff) | 0x8000000;
    *(volatile uint32_t*)0x2000f030 |= 0x1003;
    temp = *(volatile uint32_t*)0x2000f884;
    *(volatile uint32_t*)0x2000f884 = (temp & 0xf000ffff) | 0x82000f4;
    *(volatile uint32_t*)0x200010cc |= 0x10000000;
    *(volatile uint32_t*)0x2000163c = 0;
    temp = *(volatile uint32_t*)0x20001064;
    *(volatile uint32_t*)0x20001064 = (temp & 0xfffe0008) | 0x4c2c;
    temp = *(volatile uint32_t*)0x20001128;
    *(volatile uint32_t*)0x20001128 = (temp & 0xff800800) | 0x4c2491;
    temp = *(volatile uint32_t*)0x2000112c;
    *(volatile uint32_t*)0x2000112c = (temp & 0xff800800) | 0x4c24c2;
    temp = *(volatile uint32_t*)0x200010d4;
    *(volatile uint32_t*)0x200010d4 = (temp & 0xfff0f00f) | 0xf013c1;
    *(volatile uint32_t*)0x20001090 |= 0x10000;
    *(volatile uint32_t*)0x200010b8 |= 0x10;
    *(volatile uint32_t*)0x20001138 |= 3;
    temp = *(volatile uint32_t*)0x20001130;
    *(volatile uint32_t*)0x20001130 = (temp & 0xff800e92) | 0x491092;
    temp = *(volatile uint32_t*)0x2000108c;
    *(volatile uint32_t*)0x2000108c = (temp & 0xfffffff8) | 2;
    *(volatile uint32_t*)0x20001618 &= 0x3fffffff;
    *(volatile uint32_t*)0x20001220 &= 0xffffe60c;

    /*rf_pri_full_cal();
    rf_pri_gain_table_WR2REG();*/

    temp = *(volatile uint32_t*)0x20001760;
    *(volatile uint32_t*)0x20001760 = (temp & 0xff000000) | 0x3189;
    temp = *(volatile uint32_t*)0x2000175c;
    *(volatile uint32_t*)0x2000175c = (temp & 0xff000000) | 0x30f495;
    temp = *(volatile uint32_t*)0x2000179c;
    *(volatile uint32_t*)0x2000179c = (temp & 0xc00007ff) | 0xd037d000;
    temp = *(volatile uint32_t*)0x20001794;
    *(volatile uint32_t*)0x20001794 = (temp & 0xc00007ff) | 0xd06ff000;
    temp = *(volatile uint32_t*)0x2000178c;
    *(volatile uint32_t*)0x2000178c = (temp & 0xc00007ff) | 0xd077e000;
    temp = *(volatile uint32_t*)0x20001784;
    *(volatile uint32_t*)0x20001784 = (temp & 0xc00007ff) | 0x10940000;
    temp = *(volatile uint32_t*)0x2000177c;
    *(volatile uint32_t*)0x2000177c = (temp & 0xc00007ff) | 0x109c0000;
    temp = *(volatile uint32_t*)0x20001774;
    *(volatile uint32_t*)0x20001774 = (temp & 0xc00007ff) | 0x11180000;
    temp = *(volatile uint32_t*)0x2000176c;
    *(volatile uint32_t*)0x2000176c = (temp & 0xc00007ff) | 0x115c0000;
    temp = *(volatile uint32_t*)0x20001764;
    *(volatile uint32_t*)0x20001764 = (temp & 0xc00007ff) | 0x11fc0000;
    *(volatile uint32_t*)0x2000102c |= 0x4007;
    *(volatile uint32_t*)0x2000163c |= 0x8080;
}

void rfc_init() { //From rfc_init
    uint32_t temp;

    temp = *(volatile uint32_t*)0x20001220;
    *(volatile uint32_t*)0x20001220 = (temp & 0xf3ffffff) | 0x8000000;

    rf_pri_init(0, 1);

    temp = *(volatile uint32_t*)0x200011c0;
    *(volatile uint32_t*)0x200011c0 = (temp & 0xfffff000) | 0x97e; //xtal freq = 40000000

    //NOTE: values coppied from _set_rfc
    /*temp = *(volatile uint32_t*)0x200011c4;
    *(volatile uint32_t*)0x200011c4 = (temp & 0xe0000000) | (0x1fffffff & UVAR1 ???);

    temp = *(volatile uint32_t*)0x200011c8;
    *(volatile uint32_t*)0x200011c8 = (temp & 0xe0000000) | (0x1fffffff & UVAR1 ???);

    temp = *(volatile uint32_t*)0x200011cc;
    *(volatile uint32_t*)0x200011cc = (temp & 0xe0000000) | (0x1fffffff & UVAR1 ???);*/
    *(volatile uint32_t*)0x200011c4 = 81 << 22;
    *(volatile uint32_t*)0x200011c8 = 139810;
    *(volatile uint32_t*)0x200011cc = 174762;
    //

    volatile uint32_t* cal_reg = (volatile uint32_t*)0x2000113c;
}