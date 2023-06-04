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
// Assuming 16-bit instructions
uint32_t get_16_rd_register(uint32_t inst) { //rs2
    return ((inst >> 2) & 0x7) + 0x8;
}
/*uint32_t get_16_rs1_register(uint32_t inst) {
    return ((inst >> 7) & 0x7) + 0x8;
}*/
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

    data[8] = 0;
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
    uint32_t temp_val;

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

#if 0
    __handler_dump_regs(reg);
#endif

    // Read from/Write into the memory location
    if (get_inst_size( *(uint32_t *)epc ) == 2) {
        if (write) {
            temp_val = regs[get_16_rd_register(*(uint32_t *)epc)];
            //*mem_addr = temp_val;
        } else {
            //temp_val = *mem_addr;
            temp_val = 0xDEADF00D;
            reg[get_16_rd_register(*(uint32_t *)epc)] = temp_val;
        }
        _putstr("Value: ");
        _puthex((uint32_t)temp_val);
        _putstr("\r\n");
    } else {
        _putstr("Unsupported Read/write 4-byte instruction\r\n");
    }

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

