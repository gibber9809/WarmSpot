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
#define jref    void*
#define jarray  jref
#define jclass  jref
#define jbool   jint
#define vartype uint16_t

#define juint uint32_t

#define NOTYPE  0x0
#define JBYTE   0x1
#define JSHORT  0x2
#define JCHAR   0x3
#define JINT    0x4
#define JLONG   0x8
#define JFLOAT  0x10
#define JDOUBLE 0x20
#define JREF    0x40
#define JBOOL   0x80
#define JARRAY  (JREF|0x1)
#define JCLASS  (JREF|0x2)

enum JType {
    NoType  = NOTYPE,
    JByte   = JBYTE,
    JShort  = JSHORT,
    JChar   = JCHAR,
    JInt    = JINT,
    JLong   = JLONG,
    JFloat  = JFLOAT,
    JDouble = JDOUBLE,
    JRef    = JREF,
    JBool   = JBOOL,
    JArray  = JARRAY,
    JClass  = JCLASS
};

size_t jsizeof(char* jtype);
int indexsize(uint16_t jtype);

#endif