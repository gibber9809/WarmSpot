#include "class-parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "memory-management.h"
#include "endian-utils.h"



static int _parse_header(Class *class, char** file);

static int _parse_constant_pool(Class *class, char** file);

static size_t filesize(FILE* fp) {
    fseek(fp,0,SEEK_END);
    long fsize = ftell(fp);
    rewind(fp);
    return fsize;
}

int initialize_class_from_file(const char* file_name) {
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
    
    if (fread((void*)file, file_size, 1, fp) != 1) {
        //read failed
        return -1;
    }

    if (_parse_header(class, &file) < 0) {
        return -1;
    }

    if (_parse_constant_pool(class, &file) < 0) {
        return -1;
    }

    fclose(fp);

    return 0;
}

static int _parse_header(Class *class, char** file) {
    uint32_t magic;
    get4byte(&magic, (uint32_t*)*file);
    if (magic != MAGIC) {
        return -1;
    }
    *file += 4;

    get2byte(&(class->minor_version), (uint16_t*)*file);
    *file += 2;

    get2byte(&(class->major_version), (uint16_t*)*file);
    *file += 2;

    return 0;
}

static void* _skip_constant(char** constant, uint16_t* increment) {
    void* ret = (void*) *constant;
    uint8_t tag = (uint8_t)**constant;
    uint16_t len;
    *constant += 1;
    *increment = 1;
    
    switch(tag) {
        case CONSTANT_Class:
        case CONSTANT_String:
        case CONSTANT_MethodType:
            *constant += 2;
            break;
        case CONSTANT_MethodHandle:
            *constant += 3;
            break;
        case CONSTANT_Fieldref:
        case CONSTANT_Methodref:
        case CONSTANT_InterfaceMethodref:
        case CONSTANT_Integer:
        case CONSTANT_Float:
        case CONSTANT_NameAndType:
        case CONSTANT_InvokeDynamic:
            *constant += 4;
            break;
        case CONSTANT_Long:
        case CONSTANT_Double:
            *constant += 8;
            *increment = 2;
            break;
        case CONSTANT_Utf8:
            get2byte(&len,(uint16_t*)*constant);
            *constant += 2 + len;
            break;
        default:
            break;
    }

    return ret;
}

static int _parse_constant_pool(Class *class, char** file) {
    get2byte(&(class->constant_pool_count), (uint16_t*) *file);
    *file += 2;

    class->constant_pool_index = (void**) object_alloc( 
        (class->constant_pool_count-1) * sizeof(void*) );
    if (class->constant_pool_index == (void**) -1) {
        return -1;
    }

    // Set indeces to NULL pointers
    memset((void*)class->constant_pool_index, 0, 
        sizeof(void*) * (class->constant_pool_count - 1));

    // Classes are 1 indexed
    uint16_t increment = 0;
    for (uint16_t i = 1; i < class->constant_pool_count;) {
        class->constant_pool_index[i-1] = _skip_constant(file, &increment);
        
        i += increment;
    }
}
