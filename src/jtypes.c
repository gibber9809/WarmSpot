#include "jtypes.h"

#include <stdlib.h>

size_t jsizeof(char* jtype) {
    switch(*jtype) {
        case 'B':
            return sizeof(jbyte);
        case 'C':
            return sizeof(jchar);
        case 'D':
            return sizeof(jdouble);
        case 'F':
            return sizeof(jfloat);
        case 'I':
            return sizeof(jint);
        case 'J':
            return sizeof(jlong);
        case 'L':
            return sizeof(jref);
        case 'S':
            return sizeof(jshort);
        case 'Z':
            return sizeof(jbool);
        case '[':
            return sizeof(jref);
        default:
            return 0;
    }
}

int indexsize(uint16_t jtype) {
    if (jtype & (JDOUBLE | JLONG))
        return 2;

    return 1;
}