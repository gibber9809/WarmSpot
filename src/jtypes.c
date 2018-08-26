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

size_t jsizeofit(uint16_t jtype) {
    switch(jtype) {
        case JBYTE:
            return sizeof(jbyte);
        case JCHAR:
            return sizeof(jchar);
        case JDOUBLE:
            return sizeof(jdouble);
        case JFLOAT:
            return sizeof(jfloat);
        case JINT:
            return sizeof(jint);
        case JLONG:
            return sizeof(jlong);
        case JREF:
        case JARRAY:
        case JCLASS:
            return sizeof(jref);
        case JSHORT:
            return sizeof(jshort);
        case JBOOL:
            return sizeof(jbool);
        default:
            return 0;
    }
}

int indexsize(uint16_t jtype) {
    if (jtype & (JDOUBLE | JLONG))
        return 2;

    return 1;
}