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

#define JTYPEOF(T) JTYPE ## T
#define JTYPEint8_t   JBYTE
#define JTYPEint16_t  JSHORT
#define JTYPEint32_t  JINT
#define JTYPEint64_t  JLONG
#define JTYPEuint16_t JCHAR
#define JTYPEfloat    JFLOAT
#define JTYPEdouble   JDOUBLE

#define JINDEXSIZE(T) JINDEX ## T
#define JINDEXint8_t   1
#define JINDEXint16_t  1
#define JINDEXint32_t  1
#define JINDEXint64_t  2
#define JINDEXuint16_t 1
#define JINDEXfloat    1
#define JINDEXdouble   2

#define JSIZEOF(T) JSIZE ## T
#define JSIZEint8_t   1
#define JSIZEint16_t  2
#define JSIZEint32_t  4
#define JSIZEint64_t  8
#define JSIZEuint16_t 2
#define JSIZEfloat    4
#define JSIZEdouble   8

size_t jsizeof(char* jtype);
size_t jsizeofit(uint16_t jtype);
int indexsize(uint16_t jtype);

#endif