#include "class-parser.h"
#include "memory-management.h"
#include "cpu.h"

int main(int argc, const char *argv[]) {
    Cpu* initial_cpu;
    Class* initial_class;
    
    if (argc >= 3) {
        dyn_mem_init();
        jcl_path = argv[2];
        create_class_from_file(&initial_class, argv[1]);

        initial_cpu = init_first_cpu(initial_class, argc - 3, &argv[3]);
    }

    return 0;
}