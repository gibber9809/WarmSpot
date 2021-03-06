#ifndef MEMORY_MANAGEMENT_H
#define MEMORY_MANAGEMENT_H
#include <stdlib.h>
#include <stdint.h>

#define ALIGN(addr) ((addr&0x07)==0 ? addr : (addr&(-1<<3)) + 0x08)
#define INITIAL_HEAP_SIZE 0x200000

struct Object {
    struct Object* next;
    struct Object* prev;
    size_t size;
};
typedef struct Object Object;

struct ClassMem {
    struct ClassMem* next;
    size_t size;
};
typedef struct ClassMem ClassMem;

typedef struct {
    long heap_start;
    long dyn_object_start;
    long heap_end;
    Object* first_object;
    Object* last_object;
    ClassMem* first_class;
    ClassMem* last_class;
} HeapState;

extern HeapState heap;


void dyn_mem_init();

void* class_alloc(size_t size);
void* object_alloc(size_t size);
void object_free(void* object);

uint16_t align(uint16_t offset, size_t bytes);
#endif