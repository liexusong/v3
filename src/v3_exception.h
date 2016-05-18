#ifndef _V3_EXCEPTION_H
#define _V3_EXCEPTION_H 

v3_base_object_t * Error(v3_ctx_t *ctx);
extern v3_object_t      *Error_prototype;
extern v3_object_t      *TypeError_prototype;

void v3_set_error(v3_ctx_t *ctx, v3_object_t *error);
extern v3_object_t *v3_SyntaxError;
extern v3_object_t *v3_TypeError;
extern v3_object_t *v3_ReferenceError;
#endif