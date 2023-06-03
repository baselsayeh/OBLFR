#include "rx.h"

// These have special address / alignment requirements since they are used by DMA & WiFi hardware presumably?
// HACK: right now we put everything into the wifi sram
struct rx_dmadesc rx_dma_hdrdesc[13] __attribute__((aligned(16))) /*__attribute__((section(".wifi_ram")))*/;
struct rx_swdesc rx_swdesc_tab[13] __attribute__((aligned(16))) /*__attribute__((section(".wifi_ram")))*/;
struct rx_payloaddesc rx_payload_desc[13] __attribute__((aligned(16)))  /*__attribute__((section(".wifi_ram")))*/;
uint8_t rx_payload_desc_buffer[13*848] __attribute__((aligned(512))) /*__attribute__((section(".wifi_ram")))*/;

struct rxl_cntrl_env_tag {
    struct list ready;
    struct rx_dmadesc *first;
    struct rx_dmadesc *last;
    struct rx_dmadesc *free;
};
struct rxl_cntrl_env_tag rxl_cntrl_env;

struct rx_hwdesc_env_tag {
    struct rx_pbd* last;
    struct rx_pbd* free;
};
struct rx_hwdesc_env_tag rx_hwdesc_env;


void wifi_rxl_init() {
    __asm volatile( "csrc mstatus, 8" );

    struct rx_dmadesc *d = NULL;
    memset(rx_dma_hdrdesc, 0, sizeof(rx_dma_hdrdesc));
    for (int i = 0; i < (sizeof(rx_dma_hdrdesc)/sizeof(struct rx_dmadesc)); i++) {
        d = rx_dma_hdrdesc + i;
        d->hd.upatternrx = 0xbaadf00d;
        d->hd.next = rx_dma_hdrdesc + i + 1;
        d->hd.swdesc = rx_swdesc_tab + i;
        rx_swdesc_tab[i].dma_hdrdesc = d;
    }
    d->hd.next = NULL;
    //blmac_rx_header_head_ptr_set(&rx_dma_hdrdesc[1]);
    *(volatile struct rx_dmadesc**)0x24b081b8 = &rx_dma_hdrdesc[1];

    struct rx_payloaddesc *pd = NULL;
    memset(rx_payload_desc, 0, sizeof(rx_payload_desc));
    const int n = sizeof(rx_payload_desc)/sizeof(struct rx_payloaddesc);
    const int bufsize = sizeof(rx_payload_desc_buffer) / n;
    uint8_t* dataptr = rx_payload_desc_buffer;
    for (int i = 0; i < n; i++) {
        pd = rx_payload_desc + i;
        pd->pbd.upattern = 0xc0dedbad;
        pd->pbd.next = (struct rx_pbd*)(rx_payload_desc + i + 1);
        pd->pbd.datastartptr = dataptr;
        pd->pbd.dataendptr = dataptr + bufsize - 1;
        pd->buffer_rx = pd->pbd.datastartptr;
        dataptr += bufsize;
    }
    pd->pbd.next = NULL;
    //blmac_rx_payload_head_ptr_set(&rx_payload_desc[1]);
    *(volatile struct rx_payloaddesc**)0x24b081bc = &rx_payload_desc[1];

    rxl_cntrl_env.first = &rx_dma_hdrdesc[1];
    rxl_cntrl_env.last = d;
    rxl_cntrl_env.free = &rx_dma_hdrdesc[0];
    rx_hwdesc_env.last = (struct rx_pbd*)pd;
    rx_hwdesc_env.free = (struct rx_pbd*)(&rx_payload_desc[0]);

    rxl_cntrl_env.ready.first = rxl_cntrl_env.ready.last = NULL;

    // signal RX header new head
    //blmac_dma_cntrl_set(0x4000000);
    *(volatile uint32_t *)0x24b08180 = 0x4000000;
    // signal RX payload new head
    //blmac_dma_cntrl_set(0x8000000);
    *(volatile uint32_t *)0x24b08180 = 0x8000000;
    __asm volatile( "csrs mstatus, 8" );
}
