#include "class-parser.h"
#include "memory-management.h"

int main() {
    dyn_mem_init();
    initialize_class_from_file("SimpleJava.class");
}