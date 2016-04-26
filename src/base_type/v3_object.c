#include <v3_core.h>
#include <v3_object.h>

v3_int_t 
v3_object_init(v3_ctx_t *ctx, v3_object_t *obj, size_t capacity)
{
    obj->base.type = V3_DATA_TYPE_OBJECT;
    obj->__proto__ = Object_prototype;
    obj->ref_count = 0;
    obj->__attrs__ = v3_dict_create(ctx->pool, capacity);    
    if (obj->__attrs__ == NULL) return V3_ERROR;
    
    return V3_OK;
}

v3_object_t *v3_object_create(v3_ctx_t *ctx, size_t capacity)
{
    v3_object_object_t  *aobj;

    aobject = v3_palloc(ctx->pool, sizeof(*aobj));
    if (aobject == NULL) return NULL;

    rc = v3_object_init(ctx, capacity);
    if (rc != V3_OK) {
        return NULL;
    }

    return obj;
}

v3_int_t 
v3_object_set(v3_ctx_t *ctx, v3_object_t* obj, v3_str_t *key, v3_base_object_t *value)
{
    //v3_base_objet_t     *evalue;
    return v3_dict_set(obj->__attrs__, key, value);
}
