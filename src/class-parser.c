#include "class-parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "memory-management.h"
#include "endian-utils.h"

const char *ConstantValue = "ConstantValue";
const char *Code = "Code";
const char *Exceptions = "Exceptions";
const char *BootsrapMethods = "BootstrapMethods";
const char *DOTCLASS = ".class";
const char* jcl_path = NULL;

static int _parse_header(Class *class, char** file);

static int _parse_constant_pool(Class *class, char** file, size_t *alloc_size);

static void _parse_class_info(Class *class, char** file);

static void _parse_interfaces(Class *class, char** file, size_t *alloc_size);

static int _parse_fields(Class *class, char** file, size_t *alloc_size);

static int _parse_methods(Class *class, char** file, size_t *alloc_size);

static int _parse_class_attributes(Class *class, char** file, size_t *alloc_size);

static void* _skip_constant(char** constant, uint16_t* increment, size_t* alloc_size);

static void* _skip_attribute_info(char** attribute, size_t* alloc_size);

static void* _skip_field_or_method(char** file, Class* class, size_t* alloc_size, int nattr, ...);

static int _relocate_class(Class **classref, size_t alloc_size, size_t index_offset);

static bool _class_infocmp(Class* class1, char* classinfo1, Class* class2, char* classinfo2);

static Class* _find_class_from_string(char* str);

static size_t filesize(FILE* fp) {
    fseek(fp,0,SEEK_END);
    long fsize = ftell(fp);
    rewind(fp);
    return fsize;
}

char* get_const(Class* class, uint16_t constant_index) {
    //assert
    return (char*) class->constant_pool_index[constant_index - 1];
}

void get_method(Class* class, uint16_t method_index, MethodInfo* rmethod) {
    uint16_t* method = class->method_index[method_index];
    
    get2byte(&(rmethod->access_flags), method);
    get2byte(&(rmethod->name_index), &method[1]);
    get2byte(&(rmethod->descriptor_index), &method[2]);
    get2byte(&(rmethod->attributes_count), &method[3]);
    
    if (rmethod->attributes_count > 0)
        rmethod->attributes = (uint8_t*) &method[4];
    else
        rmethod->attributes = NULL;
}

Class* find_class_from_class_info(Class* class, char* class_info, void* loader) {
    ClassMem* cur_classmem = heap.first_class;
    Class* cur_class;
    char* cur_class_info;

    while (cur_classmem != NULL) {
        cur_class = (Class*) (((char*)cur_classmem) + sizeof(ClassMem));
        cur_class_info = get_const(cur_class, cur_class->this_class);

        if (cur_class->initiating_loader == loader 
            && _class_infocmp(class, class_info, cur_class, cur_class_info))
            return cur_class;

        cur_classmem = cur_classmem->next;
    }

    return NULL;
}

static Class* _find_class_from_string(char* str) {
    ClassMem* cur_classmem = heap.first_class;
    Class* cur_class;
    char* cur_class_info;
    size_t len = strlen(str);
    uint16_t descriptor_len, descriptor_index;
    char* descriptor;

    while (cur_classmem != NULL) {
        cur_class = (Class*) (((char*)cur_classmem) + sizeof(ClassMem));
        cur_class_info = get_const(cur_class, cur_class->this_class);
        get2byte(&descriptor_index, (uint16_t*) &cur_class_info[1]);
        descriptor = get_const(cur_class, descriptor_index);
        get2byte(&descriptor_len, (uint16_t*) &descriptor[1]);

        if (cur_class->initiating_loader == NULL
            && descriptor_len == len
            && (memcmp((void*)str,(void*)&descriptor[3] , len) == 0))
            return cur_class;

        cur_classmem = cur_classmem->next;
    }

    return NULL;
}

int initialize_class(Class* class) {
    // TODO: implement class initialization
    return 0;
}

int create_class_from_string(Class** rclass, char* str) {
    int rc;
    size_t jcl_path_len, len, dotclass_len;
    char* path;
    Class* dummy_rclass;
    if (rclass == NULL) rclass = &dummy_rclass;
    
    if ((*rclass = _find_class_from_string(str)) != NULL) 
        return 0;
    
    len = strlen(str);
    jcl_path_len = strlen(jcl_path);
    dotclass_len = strlen(DOTCLASS);

    path = object_alloc(jcl_path_len + len + dotclass_len + 1);

    memcpy((void*)path, (void*) jcl_path, jcl_path_len);
    memcpy((void*)path + jcl_path_len, str, len);
    memcpy((void*)path + jcl_path_len + len, DOTCLASS, dotclass_len);
    path[jcl_path_len+len+dotclass_len] = '\0';

    printf("HERE IS PATH: %s\n", path);

    rc = create_class_from_file(rclass, path);
    object_free(path);

    return rc;
}

int create_class_from_const_class_info(Class** rclass, Class* class, char* class_info) {
    int rc;
    char* descriptor;
    uint16_t descriptor_index;
    uint16_t descriptor_len;
    size_t path_len;
    size_t jcl_path_len;
    size_t dotclass_len;
    char* path;
    Class* dummy_rclass;
    if (rclass == NULL) rclass = &dummy_rclass;

    // Check if the class has already been created (assume bootstrap class loader)
    if ((*rclass = find_class_from_class_info(class, class_info, NULL)) != NULL)
        return 0;

    get2byte(&descriptor_index, (uint16_t*)(class_info + 1));
    descriptor = get_const(class, descriptor_index);

    get2byte(&descriptor_len, (uint16_t*)(descriptor + 1));
    jcl_path_len = strlen(jcl_path);
    dotclass_len = strlen(DOTCLASS);
    path_len = descriptor_len + jcl_path_len + dotclass_len + 1;

    path = object_alloc(path_len);
    memcpy(path, jcl_path, jcl_path_len);
    memcpy(path + jcl_path_len, descriptor+3, descriptor_len);
    memcpy(path + jcl_path_len + descriptor_len, DOTCLASS, dotclass_len);
    path[jcl_path_len + descriptor_len + dotclass_len] = '\0';

    rc = create_class_from_file(rclass, path);
    object_free(path);

    return rc;
}

int create_class_from_file(Class** rclass, const char* file_name) {
    FILE* fp;
    size_t file_size;
    size_t alloc_size = sizeof(Class);
    size_t index_offset;
    Class* class = NULL;
    char* file = NULL;
    char* file_start = NULL;

    fp = fopen(file_name,"r");
    if (fp == NULL) return -1;
    file_size = filesize(fp);

    /*
     * Allocate space for the class on the heap. The class will
     * be parsed on the heap, and then copied to the class area
     * once the size of all of the parsed data is known.
     */
    class = (Class*) object_alloc(sizeof(Class) + file_size);
    file_start = file = ((char*)class) + sizeof(Class);
    
    if (fread((void*)file, file_size, 1, fp) != 1) {
        //read failed
        return -1;
    }

    fclose(fp);
    fp = NULL;

    if (_parse_header(class, &file) < 0) 
        return -1;

    if (_parse_constant_pool(class, &file, &alloc_size) < 0) 
        return -1;

    _parse_class_info(class, &file);

    _parse_interfaces(class, &file, &alloc_size);

    if (_parse_fields(class, &file, &alloc_size) < 0) 
        return -1;

    if (_parse_methods(class, &file, &alloc_size) < 0) 
        return -1;

    if (_parse_class_attributes(class, &file, &alloc_size) < 0)
        return -1;

    if (file - file_start != file_size)
        return -1;

    index_offset = alloc_size = ALIGN(alloc_size);
    alloc_size += sizeof(void*) 
        * (class->constant_pool_count-1 + class->fields_count + class->methods_count);

    // TODO: Figure out how much space is needed for static variables
    // Start Preparation phase by setting all of that memory to 0 in _relocate_class

    if (_relocate_class(&class, alloc_size, index_offset) < 0) {
        return -1;
    }

    // Resolve superclass
    // TODO: check for circularity
    if (class->super_class != 0)
        create_class_from_const_class_info(NULL, class, get_const(class, class->super_class));

    // Resolve superinterfaces
    
    // Mark as having bootstrap loader as defining loader, and initiating loader
    class->defining_loader = class->initiating_loader = NULL;

    if (rclass != NULL)
        *rclass = class;

    return 0;
}

static char* _find_attribute(uint16_t num_attr, Class* class, uint16_t* attribute, const char* attr_type) {
    uint16_t const_index;
    for (uint16_t i = 0; i < num_attr; ++i) {
        get2byte(&const_index, attribute);
        if (const_strcmp(attr_type, class, const_index)) {
            return (char*) attribute;
        }

        _skip_attribute_info((char**) &attribute, NULL);
    }

    return NULL;
}

static int _relocate_class(Class **classref, size_t alloc_size, size_t index_offset) {
    char *file;
    Class *class = *classref;
    Class *nclass = class_alloc(alloc_size);
    if (nclass == (void*)-1) return -1;

    file =  ((char*)nclass) + sizeof(Class);

    nclass = memcpy((void*)nclass,(void*)class,sizeof(Class));
    
    // Initialize all indexing pointers to NULL
    memset(((void*)nclass)+index_offset, 0, alloc_size - index_offset);

    nclass->constant_pool_index = 
        class->constant_pool_index != NULL ? ((void*)nclass) + index_offset : NULL;

    nclass->field_index = 
        class->field_index != NULL ? 
        ((void*)nclass) + index_offset + sizeof(void*)*(nclass->constant_pool_count - 1) : NULL;
    
    nclass->method_index =
        class->method_index != NULL ?
        ((void*)nclass) + index_offset + sizeof(void*)*(nclass->constant_pool_count - 1 + nclass->fields_count)
        : NULL;

    size_t const_size = 0;
    uint16_t increment = 0;
    for (size_t i = 1; i < class->constant_pool_count;) {
        void* const_loc = class->constant_pool_index[i-1];
        char* fconst = (char*) const_loc;
        const_size = 0;
        nclass->constant_pool_index[i-1] = (void*) file;  
        _skip_constant(&fconst, &increment, &const_size);

        memcpy((void*)file, const_loc, const_size);
        file += const_size;
        i += increment;        
    }

    if (class->interfaces_count > 0) {
        nclass->interfaces = (uint16_t*) file;
        memcpy((void*)file, class->interfaces, sizeof(uint16_t)*class->interfaces_count);
        *file += sizeof(uint16_t) * class->interfaces_count;
    }

    uint16_t num_attr = 0;
    uint32_t attr_len = 0;
    char* attr_loc;
    char* old_file;
    for (size_t i = 0; i < class->fields_count; ++i) {
        nclass->field_index[i] = (void*) file;
        old_file = ((char*)class->field_index[i]) + 8;
        memcpy((void*)file, class->field_index[i], 8);
        file += 6;
        get2byte(&num_attr, (uint16_t*)file);
        file += 2;

        if ((attr_loc = _find_attribute(num_attr, nclass, (uint16_t*)old_file, ConstantValue)) != NULL) {
            num_attr = 1;
            put2byte((uint16_t*)&file[-2],&num_attr);

            get4byte(&attr_len,(uint32_t*)&attr_loc[2]);
            memcpy((void*)file, (void*)attr_loc, 6 + attr_len);
            file += 6 + attr_len;
        }
    }

    uint16_t new_num_attr = 0;
    uint16_t* nattr_loc = NULL;
    for (size_t i = 0; i < class->methods_count; ++i) {
        nclass->method_index[i] = (void*) file;
        old_file = ((char*)class->method_index[i]) + 8;        
        memcpy((void*)file, class->method_index[i], 8);
        file += 6;
        get2byte(&num_attr, (uint16_t*)file);
        nattr_loc = (uint16_t*) file;
        file += 2;
        new_num_attr = 0;

        if ((attr_loc = _find_attribute(num_attr, nclass, (uint16_t*)old_file, Code)) != NULL) {
            new_num_attr += 1;

            get4byte(&attr_len, (uint32_t*)&attr_loc[2]);
            memcpy((void*)file, (void*)attr_loc, 6 + attr_len);
            file += 6 + attr_len;
        }

        if ((attr_loc = _find_attribute(num_attr, nclass, (uint16_t*)old_file, Exceptions)) != NULL) {
            new_num_attr += 1;
            
            get4byte(&attr_len, (uint32_t*)&attr_loc[2]);
            memcpy((void*)file, (void*)attr_loc, 6 + attr_len);
            file += 6 + attr_len;
        }

        put2byte(nattr_loc, &new_num_attr);
    }

    if (class->bootstrap_methods != NULL) {
        nclass->attributes_count = 1;
        nclass->bootstrap_methods = (void*) file;
        get4byte(&attr_len, (uint32_t*)(((char*)class->bootstrap_methods)+2));
        
        memcpy(nclass->bootstrap_methods, class->bootstrap_methods, 6 + attr_len);
    }

    // now free everything
    object_free(class->constant_pool_index);
    object_free(class->field_index);
    object_free(class->method_index);
    object_free(class);
    class = NULL;

    *classref = nclass;

    return 0;
}

static bool _class_infocmp(Class* class1, char* classinfo1, Class* class2, char* classinfo2) {
    char* utf8info1, *utf8info2;
    uint16_t utf8index1, utf8index2;

    get2byte(&utf8index1, (uint16_t*) (classinfo1+1));
    get2byte(&utf8index2, (uint16_t*) (classinfo2+1));
    utf8info1 = get_const(class1, utf8index1);
    utf8info2 = get_const(class2, utf8index2);

    return const_utf8cmp(utf8info1, utf8info2);
}

bool const_utf8cmp(char* utf8info1, char* utf8info2) {
    uint16_t len1, len2;

    get2byte(&len1, (uint16_t*)(utf8info1+1));
    get2byte(&len2, (uint16_t*)(utf8info2+1));
    
    if (len1 != len2) return false;

    if (memcmp((void*) utf8info1, (void*) utf8info2, len1 + 3) == 0)
        return true;
    
    return false;
}

bool const_strcmp(const char *str, Class *class, uint16_t constant_index) {
    const char *const_str =(const char *)class->constant_pool_index[constant_index - 1];
    uint16_t len;

    get2byte(&len, (uint16_t*)(const_str + 1));

    const_str += 3;

    for (size_t i = 0; i < len; ++i) {
        if (str[i] != const_str[i] || str[i] == '\0')
            return false;
    }

    return true;
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

static void* _skip_constant(char** constant, uint16_t* increment, size_t* alloc_size) {
    void* ret = (void*) *constant;
    uint8_t tag = (uint8_t)**constant;
    uint16_t len;
    *constant += 1;
    *alloc_size += 1;
    *increment = 1;
    
    switch(tag) {
        case CONSTANT_Class:
        case CONSTANT_String:
        case CONSTANT_MethodType:
            *constant += 2;
            *alloc_size += 2;
            break;
        case CONSTANT_MethodHandle:
            *constant += 3;
            *alloc_size += 3;
            break;
        case CONSTANT_Fieldref:
        case CONSTANT_Methodref:
        case CONSTANT_InterfaceMethodref:
        case CONSTANT_Integer:
        case CONSTANT_Float:
        case CONSTANT_NameAndType:
        case CONSTANT_InvokeDynamic:
            *constant += 4;
            *alloc_size += 4;
            break;
        case CONSTANT_Long:
        case CONSTANT_Double:
            *constant += 8;
            *alloc_size += 8;
            *increment = 2;
            break;
        case CONSTANT_Utf8:
            get2byte(&len,(uint16_t*)*constant);
            *constant += 2 + len;
            *alloc_size += 2 + len;
            break;
        default:
            break;
    }

    return ret;
}

static int _parse_constant_pool(Class *class, char** file, size_t* alloc_size) {
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
        class->constant_pool_index[i-1] = _skip_constant(file, &increment, alloc_size);
        
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

static void _parse_interfaces(Class *class, char** file, size_t* alloc_size) {
    get2byte(&(class->interfaces_count),(uint16_t*) *file);
    *file += 2;

    if (class->interfaces_count > 0)
        class->interfaces = (uint16_t*) *file;
    else
        class->interfaces = NULL;

    *alloc_size += sizeof(uint16_t) * class->interfaces_count;
    *file += sizeof(uint16_t) * class->interfaces_count;
}

static void* _skip_attribute_info(char** attribute, size_t* alloc_size) {
    uint32_t attribute_length;
    void *ret = (void*) *attribute;
    *attribute += 2;
    get4byte(&attribute_length,(uint32_t*) *attribute);
    *attribute += 4 + attribute_length;

    if (alloc_size != NULL) *alloc_size += 6 + attribute_length;

    return ret;
}

static void* _skip_field_or_method(char** file, Class* class, size_t* alloc_size, int nattr, ...) {
    void* ret = (void*) *file;
    uint16_t attributes_count;
    uint16_t const_index;
    va_list attr_keep;    
    size_t* kept_size = NULL;    
    *file += 6;
    get2byte(&attributes_count,(uint16_t*) *file);
    *file += 2;
    *alloc_size += 8;

    for (size_t i = 0; i < attributes_count; ++i) {
        kept_size = NULL;
        get2byte(&const_index, (uint16_t*) *file);
        va_start(attr_keep,nattr);
        for (int j = 0; j < nattr; ++j) {
            const char* str = va_arg(attr_keep,const char*);
            if (const_strcmp(str, class, const_index))
                kept_size = alloc_size;
        }
        _skip_attribute_info(file, kept_size);
        va_end(attr_keep);
    }

    return ret;
}

static int _parse_fields(Class *class, char** file, size_t* alloc_size) {
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
        class->field_index[i] = _skip_field_or_method(file, class, alloc_size, 
            1, ConstantValue);
    }

    return 0;
}

static int _parse_methods(Class *class, char** file, size_t* alloc_size) {
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
        class->method_index[i] = _skip_field_or_method(file, class, alloc_size, 
            2, Code, Exceptions);
    }
}

static int _parse_class_attributes(Class *class, char** file, size_t* alloc_size) {
    uint16_t const_index;
    get2byte(&(class->attributes_count),(uint16_t*) *file);
    *file += 2;

    class->bootstrap_methods = NULL;
    
    for (size_t i = 0; i < class->attributes_count; ++i) {
        get2byte(&const_index, (uint16_t*)*file);
        if (const_strcmp(BootsrapMethods, class, const_index)) {
            class->bootstrap_methods = _skip_attribute_info(file, alloc_size);
        } else {
            _skip_attribute_info(file, NULL);
        }
    }

    class->attributes_count = class->bootstrap_methods != NULL ? 1 : 0; 
}