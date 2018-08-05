#include "endian-utils.h"
#include <byteswap.h>
#include <stdint.h>

void get2byte(uint16_t* dst, uint16_t* src) {
    *dst = bswap_16(*src);
}
void get4byte(uint32_t* dst, uint32_t* src) {
    *dst = bswap_32(*src);
}
void get8byte(uint64_t* dst, uint64_t* src) {
    *dst = bswap_64(*src);
}