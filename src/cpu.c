#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "cpu.h"
#include "class-parser.h"
#include "jtypes.h"
#include "memory-management.h"

static void _set_opstack(OpstackVariable* opstack, jlong* opstack_base, char* data, uint16_t type, uint16_t index, uint16_t prev);
static uint16_t _walk_back_opstack(OpstackVariable* opstack, uint16_t start, int num);
static uint16_t _find_empty_opstack(OpstackVariable* opstack, uint16_t start, uint16_t max_index);

static void _iconstn(StackFrame* frame, jint instruction) {
    jint const_value = instruction - iconst_0;

    push_opstack(frame, (char*) &const_value, JINT, frame->opstack_top, OPSTACK_BOTTOM);
}

static void _bipush(StackFrame* frame, jbyte* byte) {
    jint value = (jint) *byte;

    push_opstack(frame, (char*) &value, JINT, frame->opstack_top, OPSTACK_BOTTOM);
}

static void _iload(StackFrame* frame, uint8_t index) {
    jint* value = (jint*) get_local_var(frame, (uint16_t) index);

    push_opstack(frame, (char*) value, JINT, frame->opstack_top, OPSTACK_BOTTOM);
}

static void _iloadn(StackFrame* frame, uint16_t instruction) {
    uint16_t index = instruction - iload_0;
    jint* load_value = (jint*) get_local_var(frame, index);

    push_opstack(frame, (char*) load_value, JINT, frame->opstack_top, OPSTACK_BOTTOM);
}

static void _istore(StackFrame* frame, uint8_t index) {
    jint* store_value = (jint*) pop_opstack(frame);

    set_local_var(frame, (uint16_t) index, (char*) store_value, JINT);
}

static void _istoren(StackFrame* frame, uint16_t instruction) {
    uint16_t index = instruction - istore_0;
    jint* store_value = (jint*) pop_opstack(frame);

    set_local_var(frame, index, (char*) store_value, JINT);
}

static void _iadd(StackFrame* frame) {
    jint* value2 = (jint*) pop_opstack(frame);
    jint* value1 = (jint*) pop_opstack(frame);
    jint result = *value1 + *value2;

    push_opstack(frame, (char*) &result, JINT, frame->opstack_top, OPSTACK_BOTTOM);
}

static void _isub(StackFrame* frame) {
    jint* value2 = (jint*) pop_opstack(frame);
    jint* value1 = (jint*) pop_opstack(frame);
    jint result = *value1 - *value2;

    push_opstack(frame, (char*) &result, JINT, frame->opstack_top, OPSTACK_BOTTOM);
}

static void _idiv(StackFrame* frame) {
    jint* value2 = (jint*) pop_opstack(frame);
    jint* value1 = (jint*) pop_opstack(frame);
    jint result = *value1 / *value2; // can throw

    push_opstack(frame, (char*) &result, JINT, frame->opstack_top, OPSTACK_BOTTOM);
}

static void _imul(StackFrame* frame) {
    jint* value2 = (jint*) pop_opstack(frame);
    jint* value1 = (jint*) pop_opstack(frame);
    jint result = *value1 * *value2;

    push_opstack(frame, (char*) &result, JINT, frame->opstack_top, OPSTACK_BOTTOM);
}

static void _irem(StackFrame* frame) {
    jint* value2 = (jint*) pop_opstack(frame);
    jint* value1 = (jint*) pop_opstack(frame);
    jint result = *value1 % *value2; // can throw

    push_opstack(frame, (char*) &result, JINT, frame->opstack_top, OPSTACK_BOTTOM);
}

static void _ineg(StackFrame* frame) {
    jint* value = (jint*) pop_opstack(frame);
    jint result = -*value;

    push_opstack(frame, (char*) &result, JINT, frame->opstack_top, OPSTACK_BOTTOM);
}

static void _ishl(StackFrame* frame) {
    jint* value2 = (jint*) pop_opstack(frame);
    jint* value1 = (jint*) pop_opstack(frame);
    jint result = *value1 << (*value2 & 0x1F);

    push_opstack(frame, (char*) &result, JINT, frame->opstack_top, OPSTACK_BOTTOM);
}

static void _ishr(StackFrame* frame) {
    jint* value2 = (jint*) pop_opstack(frame);
    jint* value1 = (jint*) pop_opstack(frame);
    jint result = *value1 >> (*value2 & 0x1F); // TODO: write tests for undefined behaviours

    push_opstack(frame, (char*) &result, JINT, frame->opstack_top, OPSTACK_BOTTOM);
}

static void _iushr(StackFrame* frame) {
    jint* value2 = (jint*) pop_opstack(frame);
    jint* value1 = (jint*) pop_opstack(frame);
    juint result = *((juint*)value1) >> (*value2 & 0x1F); // TODO: write tests for undefined behaviour

    push_opstack(frame, (char*) &result, JINT, frame->opstack_top, OPSTACK_BOTTOM);
}

static void _iand(StackFrame* frame) {
    jint* value2 = (jint*) pop_opstack(frame);
    jint* value1 = (jint*) pop_opstack(frame);
    jint result = *value1 & *value2;

    push_opstack(frame, (char*) &result, JINT, frame->opstack_top, OPSTACK_BOTTOM);
}

static void _ior(StackFrame* frame) {
    jint* value2 = (jint*) pop_opstack(frame);
    jint* value1 = (jint*) pop_opstack(frame);
    jint result = *value1 | *value2;

    push_opstack(frame, (char*) &result, JINT, frame->opstack_top, OPSTACK_BOTTOM);
}

static void _ixor(StackFrame* frame) {
    jint* value2 = (jint*) pop_opstack(frame);
    jint* value1 = (jint*) pop_opstack(frame);
    jint result = *value1 ^ *value2;

    push_opstack(frame, (char*) &result, JINT, frame->opstack_top, OPSTACK_BOTTOM);
}

static void _iinc(StackFrame* frame, uint16_t index, uint8_t constant)
{
    jint inc = (jint) *((jbyte*)&constant);
    jint* value = (jint*) get_local_var(frame, index);

    *value += inc;
}

void execute(Cpu* cpu) {
    uint8_t* code = cpu->frame->code;
    jlong* opstack_base = cpu->frame->opstack_base;
    jlong* local_vars_base = cpu->frame->local_vars_base;
    uint32_t pc = cpu->frame->pc;
    uint32_t pc_increment = 1;
    bool should_increment_pc = true;

    if (cpu->paused) return;
    printf("0x%x\n", code[pc]);

    switch(code[pc]) {
        case nop:
            break;
        case iconst_m1:
        case iconst_0:
        case iconst_1:
        case iconst_2:
        case iconst_3:
        case iconst_4:
        case iconst_5:
            _iconstn(cpu->frame, (jint) code[pc]);
            break;

        case bipush:
            _bipush(cpu->frame, (jbyte*) (&code[pc+1]));
            pc_increment = 2;
            break;

        case iload:
            _iload(cpu->frame, code[pc+1]);
            pc_increment = 2;
            break;
        
        case iload_0:
        case iload_1:
        case iload_2:
        case iload_3:
            _iloadn(cpu->frame, (uint16_t) code[pc]);
            break;

        case istore:
            _istore(cpu->frame, code[pc+1]);
            pc_increment = 2;
            break;

        case istore_0:
        case istore_1:
        case istore_2:
        case istore_3:
            _istoren(cpu->frame, (uint16_t) code[pc]);
            break;

        case iadd:
            _iadd(cpu->frame);
            break;

        case isub:
            _isub(cpu->frame);
            break;

        case idiv:
            _idiv(cpu->frame);
            break;

        case imul:
            _imul(cpu->frame);
            break;

        case irem:
            _irem(cpu->frame);
            break;

        case ineg:
            _ineg(cpu->frame);
            break;

        case ishl:
            _ishl(cpu->frame);
            break;

        case ishr:
            _ishr(cpu->frame);
            break;

        case iushr:
            _iushr(cpu->frame);
            break;

        case iand:
            _iand(cpu->frame);
            break;

        case ior:
            _ior(cpu->frame);
            break;

        case ixor:
            _ixor(cpu->frame);
            break;
        
        case iinc:
            _iinc(cpu->frame, (uint16_t) code[pc+1], code[pc+2]);
            pc_increment = 3;
            break;
        
        default:
            printf("Unimplemented Instruction: 0x%x = %u\n",code[pc], code[pc]);
            should_increment_pc = false;
            cpu->paused = true;
            break;
    }

    if (cpu->error != NULL) {
        ; // an error was produced by this operation
    }

    if (should_increment_pc) {
        cpu->frame->pc += pc_increment;
    }
}

Cpu* init_first_cpu(Class* class, int argc, const char** argv) {
    Cpu* rcpu = NULL;
    StackFrame* frame;
    jlong mock_opstack_data[2];
    OpstackVariable mock_opstack[2];
    jref ref = NULL;
    char* method_descriptor;
    MethodInfo method_info;
    uint16_t mock_opstack_prev = OPSTACK_BOTTOM;
    uint16_t mock_opstack_top = 0;
    bool main_found = false;
    bool static_method;
    // initialize the initial class, [Ljava/lang/String; class
    initialize_class(class);
    
    // Find main method
    for (uint16_t i = 0; i < class->methods_count; ++i) {
        get_method(class, i, &method_info);

        if (method_info.access_flags & ACC_PUBLIC
            && const_strcmp("main", class, method_info.name_index)
            && const_strcmp("([Ljava/lang/String;)V", class, method_info.descriptor_index)) {
                main_found = true;
                break;
        }
            
    }

    if (!main_found) return NULL;

    // if main is not static instantiate initial class
    static_method = (method_info.access_flags & ACC_STATIC) > 0;
    if (!static_method)
        ; // instantiate main class

    // instantiate the [Ljava/lang/String;
    
    // create mock opstack and push (initial class) [java/lang/String; onto it
    // for now just push a null reference and call it an array class
    _set_opstack(mock_opstack, mock_opstack_data, (char*) &ref, JARRAY, mock_opstack_top, mock_opstack_prev);

    // call new_stackframe with void main(String[]) on initial class
    frame = new_stackframe(class, &method_info, NULL);

    // call push_method_arguments with mocked stackframe and new stackframe
    method_descriptor = get_const(class, method_info.descriptor_index);    
    push_method_arguments(frame, mock_opstack, mock_opstack_data, &mock_opstack_top, method_descriptor, !static_method);

    rcpu = (Cpu*) object_alloc(sizeof(Cpu));

    rcpu->frame = frame;
    rcpu->error = NULL;
    rcpu->paused = false;

    return rcpu;
}

StackFrame* new_stackframe(Class* class, MethodInfo* method, StackFrame* prev_frame) {
    size_t alloc_size;
    StackFrame* rframe;
    Code code;

    get_code(method, &code);

    // This can be done more cleverly to reduce the size of the allocation
    alloc_size = ALIGN(sizeof(StackFrame)) + (sizeof(OpstackVariable) + sizeof(jlong)) * code.max_opstack;
    alloc_size += (sizeof(vartype) + sizeof(jlong)) * code.max_locals;

    rframe = (StackFrame*) object_alloc(alloc_size);

    rframe->code = code.code;
    rframe->exception_table = code.exc_table_len > 0 ? code.exception_table : NULL;
    rframe->exc_table_len = code.exc_table_len;
    rframe->max_locals = code.max_locals;
    rframe->max_opstack = code.max_opstack;
    rframe->opstack_top = OPSTACK_BOTTOM;
    rframe->local_vars_base =  (jlong*) (((char*)rframe) + ALIGN(sizeof(StackFrame)));
    rframe->opstack_base = &(rframe->local_vars_base[code.max_locals]);
    rframe->opstack = (OpstackVariable*) &(rframe->opstack_base[code.max_opstack]);
    rframe->local_vars = (vartype*) &(rframe->opstack[code.max_opstack]);
    rframe->prev_frame = prev_frame;
    rframe->pc = 0;
    rframe->class = class;

    memset((void*) rframe->opstack, NOTYPE, sizeof(OpstackVariable) * code.max_opstack);
    memset((void*) rframe->local_vars_base, 0, sizeof(jlong) * code.max_locals);
    memset((void*) rframe->local_vars, NOTYPE, sizeof(vartype) * code.max_locals);

    return rframe;
}

static void _skip_class_descriptor(char** descriptor) {
    /*
     * Skips a class descriptor, leaving char* descriptor pointing at 
     * the trailing semicolon
     */
    while (**descriptor != ';')
        ++(*descriptor);
}

static void _skip_array_descriptor(char** descriptor) {
    /*
     * Skips an array descriptor, leaving char* descriptor pointing 
     * to the last character of the element type
     */

     while (**descriptor == '[')
        ++(*descriptor);
    
    if (**descriptor == 'L')
        _skip_class_descriptor(descriptor);
}

static size_t _count_method_args(char* descriptor) {
    size_t nargs = 0;
    ++descriptor;
    
    /*
     * Counts the number of arguments to a method. Assumes that descriptor
     * points to the first byte of the method descriptor and that the descriptor
     * is valid.
     */

    while (*descriptor != ')') {
        switch(*descriptor) {
            case '[':
                _skip_array_descriptor(&descriptor);
                break;
            case 'L':
                _skip_class_descriptor(&descriptor);
                break;
            default:
                break;
        }
        ++nargs;
        ++descriptor;
    }

    return nargs;
}

void push_method_arguments(StackFrame* new_stackframe, OpstackVariable* prev_opstack, jlong* prev_opstack_data, 
    uint16_t* prev_opstack_top, char* method_descriptor, bool this) {
    size_t nargs = _count_method_args((char*) &method_descriptor[3]);
    if (this) ++nargs;

    for (size_t i = 0; i < nargs; ++i) {
        uint16_t arg_index = _walk_back_opstack(prev_opstack, *prev_opstack_top, nargs - i - 1);
        set_local_var(new_stackframe, arg_index, 
            (char*) (&prev_opstack_data[arg_index]), prev_opstack[arg_index].type);
        prev_opstack[arg_index].type = 0;
    }

    // Set the top of the opstack to prev of first 
    *prev_opstack_top = _walk_back_opstack(prev_opstack, *prev_opstack_top, nargs - 1);
}

jlong* get_local_var(StackFrame* frame, uint16_t index) {
    return &(frame->local_vars_base[index]);
}

void set_local_var(StackFrame* frame, uint16_t index, char* data, uint16_t type) {
    vartype* stack_vars = frame->local_vars;
    char* local_var = (char*) &(frame->local_vars_base[index]);
    
    switch(type) {
        case JByte:
            *((jbyte*)local_var) = *((jbyte*)data);
            break;
        case JShort:
            *((jshort*)local_var) = *((jshort*)data);
            break;
        case JInt:
            *((jint*)local_var) = *((jint*)data);
            break;
        case JLong:
            *((jlong*)local_var) = *((jlong*)data);
            stack_vars[index+1] = NOTYPE;
            break;
        case JBool:
            *((jbool*)local_var) = *((jbool*)data);
            break;
        case JFloat:
            *((jfloat*)local_var) = *((jfloat*)data);
            break;
        case JDouble:
            *((jdouble*)local_var) = *((jdouble*)data);
            stack_vars[index+1] = NOTYPE;
            break;
        case JRef:
            *((jref*)local_var) = *((jref*)data);
            break;
        case JArray:
            *((jarray*)local_var) = *((jarray*)data);
            break;
        case JClass:
            *((jclass*)local_var) = *((jclass*)data);
            break;
        default:
            break;
    }
    stack_vars[index] = type;
}

static void _set_opstack(OpstackVariable* opstack, jlong* opstack_base, char* data, uint16_t type, uint16_t index, uint16_t prev) {
    char* var = (char*) &opstack_base[index];
    switch(type) {
        case JByte:
            *((jbyte*)var) = *((jbyte*)data);
            break;
        case JShort:
            *((jshort*)var) = *((jshort*)data);
            break;
        case JInt:
            *((jint*)var) = *((jint*)data);
            break;
        case JLong:
            *((jlong*)var) = *((jlong*)data);
            break;
        case JBool:
            *((jbool*)var) = *((jbool*)data);
            break;
        case JFloat:
            *((jfloat*)var) = *((jfloat*)data);
            break;
        case JDouble:
            *((jdouble*)var) = *((jdouble*)data);
            break;
        case JRef:
            *((jref*)var) = *((jref*)data);
            break;
        case JArray:
            *((jarray*)var) = *((jarray*)data);
            break;
        case JClass:
            *((jclass*)var) = *((jclass*)data);
            break;
        default:
            break;
    }

    opstack[index].type = type;
    opstack[index].prev = prev;
}

jlong* pop_opstack(StackFrame* frame) {
    uint16_t index = frame->opstack_top;
    jlong* ref = &(frame->opstack_base[index]);
    frame->opstack[index].type = NOTYPE;
    frame->opstack_top = frame->opstack[index].prev;

    return ref;
}

uint16_t push_opstack(StackFrame* frame, char* data, uint16_t type, uint16_t after, uint16_t next) {
    uint16_t index;
    if (after == OPSTACK_BOTTOM && next == OPSTACK_BOTTOM) {
        index = 0;
        _set_opstack(frame->opstack, frame->opstack_base, data, type, 0, after);
        frame->opstack_top = 0;
    } else if (after == frame->opstack_top) {
        index = _find_empty_opstack(frame->opstack, after, frame->max_opstack);
        _set_opstack(frame->opstack, frame->opstack_base, data, type, index, after);
        frame->opstack_top = index;
    } else if (next != OPSTACK_BOTTOM) {
        // In this case we are inserting into somewhere not the top of the stack
        index = _find_empty_opstack(frame->opstack, frame->opstack_top, frame->max_opstack);
        _set_opstack(frame->opstack, frame->opstack_base, data, type, index, after);
        frame->opstack[next].prev = index;
    }
    // We should always hit one of the above three cases, otherwise we are in an error state

    return index;
}

static uint16_t _walk_back_opstack(OpstackVariable* opstack, uint16_t start, int num) {
    /*
     * Returns the index holding the element num before start, if that would lead the
     * result to be below the first item on the opstack OPSTACK_BOTTOM is returned.
     */
    
    while (num > 0 && start != OPSTACK_BOTTOM) {
        start = opstack[start].prev;
        --num;
    }

    return start;
}

static uint16_t _find_empty_opstack(OpstackVariable* opstack, uint16_t start, uint16_t max_index) {
    uint16_t cur_index = start;
    while (cur_index < max_index) {
        if (opstack[cur_index].type == 0)
            return cur_index;
        
        ++cur_index;
    }

    cur_index = start;
    do {
        --cur_index;
        
        if (opstack[cur_index].type == 0)
            return cur_index;
    } while (cur_index != 0);

    // It isn't possible to reach here during normal execution because
    // we know the size of the opstack statically, and have allocated enough
    // space for it.
    return OPSTACK_BOTTOM;
}