#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#include "class-parser.h"
#include "jtypes.h"

#define OPSTACK_BOTTOM 0xFFFF
typedef struct {
    uint16_t prev;
    vartype type;
} OpstackVariable;

struct StackFrame {
    struct StackFrame* prev_frame;
    uint8_t* code;
    uint16_t* exception_table;
    jlong* local_vars_base;
    jlong* opstack_base;
    Class* class;
    vartype* local_vars;
    OpstackVariable* opstack;
    uint32_t pc;
    uint16_t max_locals;
    uint16_t max_opstack;
    uint16_t opstack_top;
    uint16_t exc_table_len;
};
typedef struct StackFrame StackFrame;

typedef struct {
    StackFrame* frame;
    void* error;
} Cpu;

void execute(Cpu* cpu);

Cpu* init_first_cpu(Class* class, int argc, const char** argv);
StackFrame* new_stackframe(Class* class, MethodInfo* method, StackFrame* prev_frame);
void push_method_arguments(StackFrame* new_stackframe, OpstackVariable* prev_opstack, jlong* prev_opstack_data, uint16_t* prev_opstack_top, char* method_descriptor, bool this);
void set_local_var(StackFrame* frame, uint16_t index, char* data, uint16_t type);
uint16_t push_opstack(StackFrame* frame, char* data, uint16_t type, uint16_t after, uint16_t next);

enum Opcodes {
    nop = 0x0
};


#endif