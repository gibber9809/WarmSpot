#ifndef MEMORY_MANAGEMENT_H
#define MEMORY_MANAGEMENT_H
#include <stdlib.h>

#define ALIGN(addr) (addr&0x07)==0 ? addr : addr + 0x08 
#define INITIAL_HEAP_SIZE 0x200000

typedef struct {
    long heap_start;
    long dyn_object_start;
    long heap_end;
} HeapState;


void dyn_mem_init();

void* class_alloc(size_t size);
void* object_alloc(size_t size);
#endif