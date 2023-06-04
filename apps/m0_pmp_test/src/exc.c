/* Default exception entry */
#include <csi_core.h>
#include <risc-v/csr.h>

//Instruction length
uint32_t get_inst_size(uint32_t inst) {
    if ((inst & 0x3) == 0) //least two significant bits are zeros: 16 bit
        return 2;

    //
    //inst >>= 2;

    //No more checks for M0
    return 4;
}
//

///////
#include <bflb_uart.h>
extern struct bflb_device_s *console;
void _putchar(char c) {
    int ret = -1;

    while ((ret = bflb_uart_putchar(console, c)) == -1) {
        bflb_mtimer_delay_ms(1);
    }

    //return ret;
    return;
}
void _putstr(char *str) {
    for (; *str != 0x00; str++)
        _putchar(*str);

    return;
}
uint8_t __hex_vals[] = "0123456789abcdef";
void _puthex(uint32_t val) {
    uint8_t data[9];

    data[9] = 0;
    for (uint32_t i=0; i<8; i++) {
        data[i] = __hex_vals[(val>>(28-(i*4)))&0xf];
    }

    _putstr((char *)data);

    return;
}
/////////////////////

//Descriptions can be found in irq_ctx.h
void __handler_dump_regs(uint32_t *regs) {
    //////
    _putstr("*****\r\n");
    for (uint32_t i=0; i<34; i++) {
       _putstr("Reg (");
       _puthex(i);
       _putstr("): ");
       _puthex(regs[i]);
       _putstr("\r\n");
    }
    _putstr("*****\r\n\r\n");
    //////
}

//void __pmp_write_exception(uintptr_t *regs) {
void __pmp_exception(uintptr_t *regs, bool write) {
    //unsigned long cause;
    unsigned long epc;
    unsigned long tval;
    uint32_t *reg = (uint32_t *)regs;

    uint32_t *mem_addr;

    //cause = READ_CSR(CSR_MCAUSE);
    epc = READ_CSR(CSR_MEPC);
    tval = READ_CSR(CSR_MTVAL);

    mem_addr = (uint32_t *)tval;

    _putstr("* PMP ");
    if (write == true)
        _putstr("Write");
    else
        _putstr("Read");
    _putstr(" Exception, Address:");
    _puthex((uint32_t)tval);
    _putstr(", PC:");
    _puthex((uint32_t)epc);
    _putstr("\r\n");

#if 1
    __handler_dump_regs(reg);
#endif

    // Read from/Write into the memory location
    /*if (write)
        *mem_addr = 0xDDDDDDDD;
    else
        reg[1] = *mem_addr;*/

    //Advance PC so that it does not return
    //PC reg is at 0
    //reg[0] += 2;
    reg[0] += get_inst_size( *(uint32_t *)epc );
}

void __dump_exception_entry(uintptr_t *regs);
//void exception_entry(uintptr_t *regs) {
void exception_entry(uintptr_t *regs) {
    unsigned long cause;

    cause = READ_CSR(CSR_MCAUSE);

    /*if (((int)cause) != 0x38000007) {
        __dump_exception_entry(regs);
        return;
    }*/

    switch ((int)cause) {
        case 0x38000007: //PMP write
        case 0x38000005: //PMP Read
            __pmp_exception(regs, ((int)cause)==0x38000007?true:false );
            //__pmp_read_exception(regs);
            break;

        default:
            __dump_exception_entry(regs);
            break;
    }
}

