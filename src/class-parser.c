#include "class-parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "memory-management.h"
#include "endian-utils.h"



static int _parse_header(Class *class, char** file);

static int _parse_constant_pool(Class *class, char** file);

static void _parse_class_info(Class *class, char** file);

static void _parse_interfaces(Class *class, char** file);

static int _parse_fields(Class *class, char** file);

static int _parse_methods(Class *class, char** file);

static int _parse_class_attributes(Class *class, char** file);

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

    fclose(fp);
    fp = NULL;

    if (_parse_header(class, &file) < 0) {
        return -1;
    }

    if (_parse_constant_pool(class, &file) < 0) {
        return -1;
    }

    _parse_class_info(class, &file);

    _parse_interfaces(class, &file);

    if (_parse_fields(class, &file) < 0) {
        return -1;
    }

    if (_parse_methods(class, &file) < 0) {
        return -1;
    }

    if (_parse_class_attributes(class, &file) < 0) {
        return -1;
    }

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

    // Set indices to NULL pointers
    memset((void*)class->constant_pool_index, 0, 
        sizeof(void*) * (class->constant_pool_count - 1));

    // Classes are 1 indexed
    uint16_t increment = 0;
    for (uint16_t i = 1; i < class->constant_pool_count;) {
        class->constant_pool_index[i-1] = _skip_constant(file, &increment);
        
        i += increment;
    }
}

static void _parse_class_info(Class *class, char** file) {
    get2byte(&(class->access_flags),(uint16_t*) *file);
    *file += 2;

    get2byte(&(class->this_class),(uint16_t*) *file);
    *file += 2;

    get2byte(&(class->super_class),(uint16_t*) *file);
    *file += 2;
}

static void _parse_interfaces(Class *class, char** file) {
    get2byte(&(class->interfaces_count),(uint16_t*) *file);
    *file += 2;

    if (class->interfaces_count > 0)
        class->interfaces = (uint16_t*) *file;
    else
        class->interfaces = NULL;

    *file += sizeof(uint16_t) * class->interfaces_count;
}

static void* _skip_attribute_info(char** attribute) {
    uint32_t attribute_length;
    void *ret = (void*) *attribute;
    *attribute += 2;
    get4byte(&attribute_length,(uint32_t*) *attribute);
    *attribute += 4 + attribute_length;

    return ret;
}

static void* _skip_field_or_method(char** file) {
    void* ret = (void*) *file;
    uint16_t attributes_count;
    *file += 6;
    get2byte(&attributes_count,(uint16_t*) *file);
    *file += 2;

    for (size_t i = 0; i < attributes_count; ++i) {
        _skip_attribute_info(file);
    }
}

static int _parse_fields(Class *class, char** file) {
    get2byte(&(class->fields_count),(uint16_t*) *file);
    *file += 2;

    if (class->fields_count > 0)
        class->field_index = (void**) object_alloc(
            sizeof(void*) * class->fields_count );
    else
        class->field_index = NULL;

    if (class->field_index == (void**)-1)
        return -1;
    
    for (size_t i = 0; i < class->fields_count; ++i) {
        class->field_index[i] = _skip_field_or_method(file);
    }

    return 0;
}

static int _parse_methods(Class *class, char** file) {
    get2byte(&(class->methods_count),(uint16_t*) *file);
    *file += 2;

    if (class->methods_count > 0)
        class->method_index = (void**) object_alloc(
            sizeof(void*) * class->methods_count );
    else
        class->method_index = NULL;

    if (class->method_index == (void**) -1)
        return -1;
    
    for (size_t i = 0; i < class->methods_count; ++i) {
        class->method_index[i] = _skip_field_or_method(file);
    }
}

static int _parse_class_attributes(Class *class, char** file) {
    get2byte(&(class->attributes_count),(uint16_t*) *file);
    *file += 2;

    if (class->attributes_count > 0)
        class->attribute_index = (void**) object_alloc(
            sizeof(void*) * class->attributes_count );
    else
        class->attribute_index = NULL;
    
    if (class->attribute_index == (void**) -1)
        return -1;
    
    for (size_t i = 0; i < class->attributes_count; ++i) {
        class->attribute_index[i] = _skip_attribute_info(file);
    }
}