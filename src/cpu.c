#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "cpu.h"
#include "class-parser.h"
#include "jtypes.h"
#include "memory-management.h"

Cpu* init_first_cpu(Class* class, int argc, const char** argv) {
    StackFrame* frame;
    void* mock_opstack_data[2];
    StackVariable mock_opstack[2];
    MethodInfo method_info;
    uint16_t mock_opstack_top = 0;
    bool main_found = false;
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
    if (!(method_info.access_flags & ACC_STATIC))
        ; // instantiate main class

    // instantiate the [Ljava/lang/String;
    
    // create mock opstack and push (initial class) [java/lang/String; onto it
    // for now just push a null reference and call it an array class
    push_opstack_ref(mock_opstack, &mock_opstack_top, (char*) mock_opstack_data, NULL, JARRAY);

    // call new_stackframe with void main(String[]) on initial class
    frame = new_stackframe(class, &method_info, NULL);

    // call push_method_arguments with mocked stackframe and new stackframe
    //push_method_arguments(frame, mock_opstack, mock_opstack, (char*) mock_opstack_data, &mock_opstack_top)

    return NULL;
}

StackFrame* new_stackframe(Class* class, MethodInfo* method, StackFrame* prev_frame) {
    size_t alloc_size;
    StackFrame* rframe;
    Code code;

    get_code(method, &code);

    // This can be done more cleverly to reduce the size of the allocation
    alloc_size = ALIGN(sizeof(StackFrame)) + (sizeof(StackVariable) + sizeof(jlong)) * (code.max_stack + code.max_locals);

    rframe = (StackFrame*) object_alloc(alloc_size);

    rframe->code = code.code;
    rframe->exception_table = code.exc_table_len > 0 ? code.exception_table : NULL;
    rframe->exc_table_len = code.exc_table_len;
    rframe->max_locals = code.max_locals;
    rframe->opstack_top = 0;
    rframe->local_vars_base =  ((uint8_t*)rframe) + ALIGN(sizeof(StackFrame));
    rframe->opstack_base = rframe->local_vars_base + code.max_locals * sizeof(jlong);
    rframe->local_vars = (StackVariable*) (rframe->opstack_base + code.max_stack * sizeof(jlong));
    rframe->opstack =  rframe->local_vars + code.max_locals;
    rframe->prev_frame = prev_frame;
    rframe->pc = 0;
    rframe->class = class;

    return rframe;
}

void push_method_arguments(StackFrame* new_stackframe, StackVariable* prev_opstack, char* prev_opstack_data, uint16_t* prev_opstack_top, uint8_t* method_descriptor) {
    return;
}

void push_opstack_ref(StackVariable* opstack, uint16_t* opstack_top, char* opstack_data, jref* data, uint16_t reftype) {
    uint16_t top = *opstack_top, prev = *opstack_top - 1;
    prev = (*opstack_top > 0 && opstack[prev].type == 0) ? prev - 1 : prev;

    if (top == 0) {
        opstack[0].offset = 0;
        opstack[0].type = reftype;
    } else {
        opstack[top].type = reftype;
        opstack[top].offset = opstack[prev].offset + jsizeofit(opstack[prev].type);
        opstack[top].offset = ALIGN(opstack[top].offset);
    }

    *((jref**)(&opstack_data[opstack[top].offset])) = data;
    *opstack_top += 1;
}

void _push_opstack(StackVariable* opstack, uint16_t* opstack_top, char* opstack_data, char* data, uint16_t type, size_t nbytes, uint8_t nindeces) {
    uint16_t top = *opstack_top, prev = *opstack_top - 1;
    prev = (*opstack_top > 0 && opstack[prev].type == 0) ? prev - 1 : prev;
    
    if (top == 0) {
        opstack[0].offset = 0;
        opstack[0].type = type;
    } else {
        opstack[top].type = type;
        opstack[top].offset = opstack[prev].offset + jsizeofit(opstack[prev].type);
        opstack[top].offset = align(opstack[top].offset, nbytes);
    }

    if (nindeces == 2) {
        opstack[top+1].type = 0;
    }

    memcpy((void*) (opstack_data + opstack[top].offset), (void*) data, nbytes);
    *opstack_top += nindeces;
}
