#include "memory-management.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

HeapState heap;

void dyn_mem_init() {
    heap.heap_start = (long) sbrk(0);
    if (heap.heap_start == -1) exit(-1);
    heap.heap_start = ALIGN(heap.heap_start);
    
    heap.heap_end = heap.heap_start + INITIAL_HEAP_SIZE;
    if(brk((void*)heap.heap_start + INITIAL_HEAP_SIZE) == -1) exit(-1);
    
    heap.dyn_object_start = heap.heap_start + INITIAL_HEAP_SIZE/2;
    heap.dyn_object_start = ALIGN(heap.dyn_object_start);

    heap.first_object = heap.last_object = NULL;
    heap.first_class = heap.last_class = NULL;
}

void* class_alloc(size_t size) {
    ClassMem* class;
    size_t total_size = size + sizeof(ClassMem);

    if (heap.first_class == NULL) {
        class = heap.first_class = heap.last_class = (ClassMem*) heap.heap_start;

        if (heap.dyn_object_start - heap.heap_start < total_size) {
            heap.first_class = NULL;
            return (void*) -1;
        }

        class->next = NULL;
        class->size = size;

        return  ((void*)class) + sizeof(ClassMem);
    }
    
    class = (ClassMem*) (((void*)heap.last_class) + heap.last_class->size + sizeof(ClassMem));
    class = (ClassMem*) ALIGN((long)class);

    if (heap.dyn_object_start - (long) class < total_size)
        return (void*) -1;
    
    heap.last_class->next = class;
    class->next = NULL;
    heap.last_class = class;
    class->size = size;

    return ((void*)class) + sizeof(ClassMem);
}

void* object_alloc(size_t size) {
    // Dummy implementation
    Object* object;
    size_t total_size = size + sizeof(Object);

    if (heap.first_object == NULL) {
        heap.first_object = (Object*) heap.dyn_object_start;
        object = heap.last_object = heap.first_object;

        if (heap.heap_end - heap.dyn_object_start < total_size) {
            // Extend the heap to be long enough
            if (sbrk(total_size - (heap.heap_end - heap.dyn_object_start)) == (void*)-1) {
                heap.first_object = NULL;
                return (void*) -1;
            } else
                heap.heap_end = (long) object + total_size;
        }

        object->next = object->prev = NULL;
        object->size = size;
        
        return ((void*) object) + sizeof(Object);
    } 
    
    object = (Object*) (((void*)heap.last_object) + heap.last_object->size + sizeof(Object));
    object = (Object*) ALIGN((long)object);
    if (heap.heap_end - (long) object < total_size) {
        if (sbrk(total_size - (long)object) == (void*)-1)
            return (void*) -1;
        else 
            heap.heap_end = (long) object + total_size;
    }

    object->size = size;
    heap.last_object->next = object;
    object->prev = heap.last_object;
    object->next = NULL;
    heap.last_object = object;
    
    return ((void*) object) + sizeof(Object);
}

void object_free(void* object) {
    if (object == NULL) return;
    
    // Make object point to the Object struct for the
    // allocated memory
    Object* ostruct = (Object*) ((char*)object - sizeof(Object));

    if (ostruct->prev == NULL && ostruct->next == NULL) {
        heap.first_object = heap.last_object = NULL;
    } else if (ostruct->next == NULL) {
        heap.last_object = ostruct->prev;
        ostruct->prev->next = NULL;
    } else if (ostruct->prev == NULL) {
        heap.first_object = ostruct->next;
        ostruct->next->prev = NULL;
    } else {
        ostruct->prev->next = ostruct->next;
        ostruct->next->prev = ostruct->prev;
    }
}

uint16_t align(uint16_t offset, size_t bytes) {
    switch(bytes) {
        case 1:
            return offset;
        case 2:
            return ((offset & 0x1) == 0 ? offset : offset + 0x1);
        case 4:
            return ((offset & 0x3) == 0 ? offset : (offset & (~0x3)) + 0x4);
        case 8:
            return ((offset & 0x7) == 0 ? offset : (offset & (~0x7)) + 0x4);
        default:
            return offset;
    }
}