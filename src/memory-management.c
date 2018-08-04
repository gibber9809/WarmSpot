#include "memory-management.h"

#include <unistd.h>
#include <stdlib.h>

HeapState heap;

void dyn_mem_init() {
    heap.heap_start = (long) sbrk(0);
    if (heap.heap_start == -1) exit(-1);
    heap.heap_start = ALIGN(heap.heap_start);
    
    heap.heap_end = heap.heap_start + INITIAL_HEAP_SIZE;
    if(brk((void*)heap.heap_start + INITIAL_HEAP_SIZE) == -1) exit(-1);
    
    heap.dyn_object_start = heap.heap_start + INITIAL_HEAP_SIZE/2;
    heap.dyn_object_start = ALIGN(heap.dyn_object_start);
}


void* class_alloc(size_t size) {
    // Dummy implementation
    return (void*)heap.heap_start;
}

void* object_alloc(size_t size) {
    // Dummy implementation
    return (void*)heap.dyn_object_start;
}