#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#include "class-parser.h"
#include "jtypes.h"

typedef struct {
    uint16_t offset;
    uint16_t type;
} StackVariable;

struct StackFrame {
    struct StackFrame* prev_frame;
    uint8_t* code;
    uint8_t* local_vars_base;
    uint8_t* opstack_base;
    Class* class;
    StackVariable* local_vars;
    StackVariable* opstack;
    uint32_t pc;
    uint16_t num_local_vars;
    uint16_t opstack_top;
};
typedef struct StackFrame StackFrame;

typedef struct {
    StackFrame* frame;
    void* error;
} Cpu;

Cpu* init_first_cpu(Class* class, int argc, const char** argv);
StackFrame* new_stackframe(Class* class, uint8_t* code_attribute, StackFrame* prev_frame);
void push_method_arguments(StackFrame* new_stackframe, StackVariable* prev_opstack, char* prev_opstack_data, uint16_t* prev_opstack_top, uint8_t* method_descriptor);
void push_opstack_ref(StackVariable* opstack, uint16_t* opstack_top, char* opstack_data, jref* data, uint16_t reftype);
void _push_opstack(StackVariable* opstack, uint16_t* opstack_top, char* opstack_data, char* data, uint16_t type, size_t nbytes, uint8_t nindeces);

#define push_opstack(opstack, opstack_top, opstack_data, data, type) \
    _push_opstack(opstack, opstack_top, opstack_data, data, JTYPEOF(type), JSIZEOF(type), JINDEXSIZE(type))

#endif