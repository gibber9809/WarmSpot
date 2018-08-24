#ifndef JTYPES_H
#define JTYPES_H

#include <stdlib.h>
#include <stdint.h>

#define jbyte   int8_t
#define jshort  int16_t
#define jint    int32_t
#define jlong   int64_t
#define jchar   uint16_t
#define jfloat  float
#define jdouble double
#define jref    int32_t
#define jbool   jint

size_t jsizeof(char* jtype);

#endif