
//void inline print_hx(uint8_t *data, uint32_t len);

//void inline print_hx(uint8_t *data, uint32_t len) {
void print_hx(uint8_t *data, uint32_t len) {
    for (int i=0; i<len; i++) {
        if (i % 16 == 0) {
            if (i != 0)
                printf("\r\n");

            printf("%08x: ", i);
        }

        printf("%02x ", data[i]);
    }
    printf("\r\n");
}
