#include <stdio.h>

#include "pretty-print.h"
#include "cpu.h"
#include "jtypes.h"

void print_opstack(OpstackVariable* opstack, uint16_t opstack_top, jlong* opstack_base) {
    printf("=== Opstack ===\n\n");

    while (opstack_top != OPSTACK_BOTTOM) {
        print_variable(&(opstack_base[opstack_top]), opstack[opstack_top].type);

        opstack_top = opstack[opstack_top].prev;
    }
    printf("\n");
}

void print_locals(vartype* locals, uint16_t max_locals, jlong* locals_base) {
    printf("=== Local Variables ===\n\n");
    
    for (uint16_t i = 0; i < max_locals; ++i) {
        print_variable(&(locals_base[i]), locals[i]);
    }
    printf("\n");
}

void print_variable(jlong* var, vartype type) {
    switch(type) {
        case JBool:
            printf("Boolean: %d\n", *((jbool*) var));
            break;
        case JByte:
            printf("Byte: %d\n", *((jbyte*) var));
            break;
        case JShort:
            printf("Short: %d\n", *((jshort*) var));
            break;
        case JInt:
            printf("Int: %d Hex: 0x%x\n", *((jint*) var), *((jint*) var));
            break;
        case JLong:
            printf("Long: %ld\n", *((jlong*) var));
            break;
        case JFloat:
            printf("Float: %f\n", *((jfloat*) var));
            break;
        case JDouble:
            printf("Double: %f\n", *((jdouble*) var));
            break;
        case JRef:
            printf("Ref: %p\n", *((jref*) var));
            break;
        case JClass:
            printf("Class: %p\n", *((jclass*) var));
            break;
        case JArray:
            printf("Array: %p\n", *((jarray*) var));
            break;
        case NoType:
            printf("NoType\n");
            break;
        default:
            printf("Unknown\n");
            break;
    }
}
