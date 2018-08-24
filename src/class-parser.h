#ifndef CLASS_PARSER_H
#define CLASS_PARSER_H

#include <stdint.h>
#include <stdbool.h>

#define MAGIC 0xCAFEBABE

#define CONSTANT_Utf8               1
#define CONSTANT_Integer            3
#define CONSTANT_Float              4
#define CONSTANT_Long               5
#define CONSTANT_Double             6
#define CONSTANT_Class              7
#define CONSTANT_String             8
#define CONSTANT_Fieldref           9
#define CONSTANT_Methodref          10
#define CONSTANT_InterfaceMethodref 11
#define CONSTANT_NameAndType        12
#define CONSTANT_MethodHandle       15
#define CONSTANT_MethodType         16
#define CONSTANT_InvokeDynamic      18

/* 
 * Common Access and Property Flags
 */

#define ACC_PUBLIC    0x0001
#define ACC_PRIVATE   0x0002
#define ACC_PROTECTED 0x0004
#define ACC_STATIC    0x0008
#define ACC_FINAL     0x0010
#define ACC_SYNTHETIC 0x1000

/*
 * Class Access and Property Flags
 */
#define ACC_SUPER      0x0020
#define ACC_INTERFACE  0x0200
#define ACC_ABSTRACT   0x0400
#define ACC_ANNOTATION 0x2000
#define ACC_ENUM       0x4000

/*
 * Field Access and Property Flags
 */

#define ACC_VOLATILE  0x0040
#define ACC_TRANSIENT 0x0080    
#define ACC_ENUM      0x4000

/*
 * Method Access and Property Flags
 * 
 * Shares public, private, protected, static final, 
 * with field flags
 */
#define ACC_SYNCHRONIZED 0x0020
#define ACC_BRIDGE       0x0040
#define ACC_VARARGS      0x0080
#define ACC_NATIVE       0x0100
#define ACC_ABSTRACT     0x0400
#define ACC_STRICT       0x0800

extern const char* jcl_path;

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
    void* bootstrap_methods;
    void* initiating_loader;
    void* defining_loader;
} Class;

int create_class_from_file(const char* file_name);

int create_class_from_class_info(Class* class, char* class_info);

char* get_const(Class* class, uint16_t constant_index);

Class* find_class_from_class_info(Class* class, char* class_info, void* loader);

bool const_utf8cmp(char* utf8info1, char* utf8info2);

bool const_strcmp(const char *str, Class* class, uint16_t constant_index);
#endif