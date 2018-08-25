#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#include "class-parser.h"

typedef struct {
    uint16_t offset;
    uint16_t type;
} StackVariable;

struct StackFrame {
    struct StackFrame* prev_frame;
    uint8_t* code;
    uint8_t* local_vars_base;
    uint8_t* op_stack_base;
    Class* class;
    StackVariable* local_vars;
    StackVariable* op_stack;
    uint32_t pc;
    uint16_t num_local_vars;
    uint16_t op_stack_top;
};
typedef struct StackFrame StackFrame;

typedef struct {
    StackFrame* frame;
    void* error;
} Cpu;

Cpu* init_first_cpu(Class* class, int argc, const char** argv);
StackFrame* new_stackframe(Class* class, uint8_t* code_attribute);
void push_method_arguments(StackFrame* new_stackframe, StackVariable* prev_op_stack, uint16_t* prev_op_stack_top, uint8_t* method_descriptor);
#endif