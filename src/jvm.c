#include "class-parser.h"
#include "memory-management.h"

int main(int argc, const char *argv[]) {
    if (argc == 3) {
        dyn_mem_init();
        jcl_path = argv[2];
        create_class_from_file(argv[1]);
    }

    return 0;
}