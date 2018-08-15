#ifndef ENDIAN_UTILS_H
#define ENDIAN_UTILS_H

#include <stdint.h>

void get2byte(uint16_t* dst, uint16_t* src);
void get4byte(uint32_t* dst, uint32_t* src);
void get8byte(uint64_t* dst, uint64_t* src);

#define put2byte(dst,src) get2byte(dst,src)
#define put4byte(dst,src) get4byte(dst,src)
#define put8byte(dst,src) get8byte(dst,src)

#endif