#ifndef CLASS_PARSER_H
#define CLASS_PARSER_H

#include <stdint.h>

typedef struct {
    uint16_t minor_version;
    uint16_t major_version;
    uint16_t constant_pool_count;
    uint16_t access_flags;
    uint16_t this_class;
    uint16_t super_class;
    uint16_t interfaces_count;
    uint16_t fields_count;
    uint16_t methods_count;
    uint16_t attributes_count;
    void** constant_pool_index;
    uint16_t* interfaces;
    void** field_index;
    void** method_index;
    void** attribute_index;
} Class;

void initialize_class_from_file(const char* file_name);

#endif