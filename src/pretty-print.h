#ifndef PRETTY_PRINT_H
#define PRETTY_PRINT_H

#include "cpu.h"
#include "jtypes.h"

void print_opstack(OpstackVariable* opstack, uint16_t opstack_top, jlong* opstack_base);
void print_locals(vartype* locals, uint16_t max_locals, jlong* locals_base);

void print_variable(jlong* var, vartype type);

#endif