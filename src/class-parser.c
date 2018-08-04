#include "class-parser.h"
#include <stdio.h>
#include <stdlib.h>

#include "memory-management.h"

static size_t filesize(FILE* fp) {
    fseek(fp,0,SEEK_END);
    long fsize = ftell(fp);
    rewind(fp);
    return fsize;
}

void initialize_class_from_file(const char* file_name) {
    FILE* fp = fopen(file_name,"r");
    size_t file_size = filesize(fp);
    Class* class = NULL;
    char* file = NULL;

    /*
     * Allocate space for the class on the heap. The class will
     * be parsed on the heap, and then copied to the class area
     * once the size of all of the parsed data is known.
     */
    class = (Class*) object_alloc(sizeof(Class) + file_size);
    file = ((char*)class) + sizeof(Class);

    fclose(fp);
}
