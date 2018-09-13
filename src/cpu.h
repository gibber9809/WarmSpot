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
    jlong rvalue;
    vartype rtype;
    bool paused;
} Cpu;

void execute(Cpu* cpu);

Cpu* init_first_cpu(Class* class, int argc, const char** argv);
StackFrame* new_stackframe(Class* class, MethodInfo* method, StackFrame* prev_frame);
void push_method_arguments(StackFrame* new_stackframe, OpstackVariable* prev_opstack, jlong* prev_opstack_data, uint16_t* prev_opstack_top, char* method_descriptor, bool this);
void set_local_var(StackFrame* frame, uint16_t index, char* data, uint16_t type);
jlong* get_local_var(StackFrame* frame, uint16_t index);
uint16_t push_opstack(StackFrame* frame, char* data, uint16_t type, uint16_t after, uint16_t next);
jlong* pop_opstack(StackFrame* frame);

enum Opcodes {
    nop = 0x0,

    iconst_m1 = 0x2,
    iconst_0 = 0x3,
    iconst_1 = 0x4,
    iconst_2 = 0x5,
    iconst_3 = 0x6,
    iconst_4 = 0x7,
    iconst_5 = 0x8,

    bipush = 0x10,

    iload = 0x15,

    iload_0 = 0x1a,
    iload_1 = 0x1b,
    iload_2 = 0x1c,
    iload_3 = 0x1d,

    istore = 0x36,

    istore_0 = 0x3b,
    istore_1 = 0x3c,
    istore_2 = 0x3d,
    istore_3 = 0x3e,

    iadd = 0x60,

    isub = 0x64,

    idiv = 0x6c,

    imul = 0x68,

    irem = 0x70,

    ineg = 0x74,

    ishl = 0x78,

    ishr = 0x7a,

    iushr = 0x7c,

    iand = 0x7e,

    ior = 0x80,

    ixor = 0x82,

    iinc = 0x84
};


#endif