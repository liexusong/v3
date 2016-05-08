#ifndef _V3_FUNCTION_OBJECT_H
#define _V3_FUNCTION_OBJECT_H 1

#include <v3_core.h>
#include <v3_base_type.h>

typedef struct {
    size_t          len; 
    v3_vector_t     *args;
} v3_arguments_t;

#define v3_function_set_prototype(ctx, function, prototype) \
    v3_obj_set(function, v3_strobj("prototype"), prototype); \
    v3_obj_set(prototype, v3_strobj("constructor"), function);

#define v3_obj_set_native_func(ctx, obj, func_name, arg_count) \
    v3_obj_set(obj, v3_strobj(""func_name""), \
    v3_function_create_native(ctx, v3_strobj(""func_name""), v3_numobj(arg_count), func_name))

typedef v3_base_object_t* (*v3_func_pt)(v3_ctx_t *ctx, v3_base_object_t *self, v3_arguments_t *args);

struct v3_function_object_s {
    v3_object_t         base;
    v3_string_object_t  *name;  
    v3_number_object_t  *length;    // args count
    unsigned int        is_native;
    v3_func_pt          native_func;
    // v3_function_block_t function;
};

extern v3_object_t             *Function_prototype;

v3_function_object_t *
v3_function_create_native(v3_ctx_t *ctx, v3_string_object_t *name, v3_number_object_t *arg_count, v3_func_pt pt);
#endif
