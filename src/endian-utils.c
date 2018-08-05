#include "endian-utils.h"
#include <byteswap.h>
#include <stdint.h>

static void _slow_get2byte(char* dst, char* src);
static void _slow_get4byte(char* dst, char* src);
static void _slow_get8byte(char* dst, char* src);


void get2byte(uint16_t* dst, uint16_t* src) {
    if ( ((long)dst) & 0x01 == 0 && ((long)src) & 0x01 == 0) 
        *dst = bswap_16(*src);
    else
        _slow_get2byte((char*) dst, (char*) src);
}
void get4byte(uint32_t* dst, uint32_t* src) {
    if ( ((long)dst) & 0x03 == 0 && ((long)src)& 0x03 == 0)
        *dst = bswap_32(*src);
    else
        _slow_get4byte((char*) dst, (char*) src);
}
void get8byte(uint64_t* dst, uint64_t* src) {
    if( ((long)dst) & 0x07 == 0 && ((long)src) & 0x07 == 0)
        *dst = bswap_64(*src);
    else
        _slow_get8byte((char*) dst, (char*) src);
}


/*
 * Byte by byte transfer because either dst or src
 * is not aligned.
 */
static void _slow_get2byte(char* dst, char* src) {
    dst[0] = src[1];
    dst[1] = src[0];
}
static void _slow_get4byte(char* dst, char* src) {
    dst[0] = src[3];
    dst[1] = src[2];
    dst[2] = src[1];
    dst[3] = src[0];
}
static void _slow_get8byte(char* dst, char* src) {
    dst[0] = src[7];
    dst[1] = src[6];
    dst[2] = src[5];
    dst[3] = src[4];
    dst[4] = src[3];
    dst[5] = src[2];
    dst[6] = src[1];
    dst[7] = src[0];
}