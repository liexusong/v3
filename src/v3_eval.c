#include <v3_core.h>
#include "v3_eval.h"
#include "v3_parser.h"
#include <assert.h>

#define V3_EVAL_NODE(node, frame) \
    v3_evaleators[node->type](ctx, frame, node)
#define V3_SET_RETURN(_name, _value) \
    ctx->ret.name = (_name); \
    ctx->ret.value = (v3_base_object_t *)(_value)
// static v3_object_t v3_global;



extern v3_int_t v3_init_Function(v3_ctx_t *ctx);
extern v3_int_t v3_init_Object(v3_ctx_t *ctx);
extern v3_int_t v3_init_Number(v3_ctx_t *ctx);

static v3_base_object_t *v3_find_property(v3_base_object_t *owner, v3_string_object_t *key);
// typedef void (*v3_disp_result_pt)(v3_ctx_t *ctx, v3_base_object_t *value);

v3_eval_pt v3_evaleators[] = {
    NULL, // V3_SYNTAX_ASSIGNMENT_EXPR,
    v3_call_expr_eval, // V3_SYNTAX_CALL_EXPR,
    v3_identifier_eval,//V3_SYNTAX_IDENTIFIER,
    v3_literal_eval,//V3_SYNTAX_LITERAL,
    NULL,//V3_SYNTAX_MEMBER_EXPR,
    v3_program_eval,//V3_SYNTAX_PROGRAM,
    NULL,//V3_SYNTAX_SEQUENCE_EXPR,
    v3_variable_statement_eval, // V3_SYNTAX_VARIABLE_DECLARATION,
    v3_variable_declarator_eval,// V3_SYNTAX_VARIABLE_DECLARATOR,
    v3_expr_statement_eval,     // V3_SYNTAX_EXPR_STATEMENT = 9,   // 
    v3_new_expr_eval,     // V3_SYNTAX_NEW_EXPR = 10,   // 
    v3_block_statement_eval,
    v3_function_declaration_eval, 
};


v3_int_t v3_init_global(v3_ctx_t *ctx)
{
    // init global;
    v3_object_t     *global;
    v3_int_t        rc;

    global = v3_palloc(ctx->pool, sizeof(v3_object_t));
    if (global == NULL) return V3_ERROR;

    rc = v3_object_init(ctx, global, 1000);
    if (rc != V3_OK) return rc;

    ctx->global = global;

    // builtin
    v3_init_Function(ctx);
    v3_init_Object(ctx);
    v3_init_Number(ctx);

    return V3_OK;
}

v3_int_t v3_eval(v3_ctx_t *ctx, char *code)
{
    v3_int_t            rc;
    v3_program_node_t   *program;
    size_t              len;
    v3_base_object_t        *ret, *owner;
    v3_string_object_t      *str;
    char                    buf[1024];
    v3_function_object_t    *func;

    len = strlen(code);
    rc = v3_parse(ctx, code, len, &program);
    if (rc != V3_OK) {
        if (ctx->err != NULL) {
            printf("%s", ctx->err);
            ctx->err = NULL;
        } else {
            printf("Unkown error %d", rc);
        }
        printf(" \n");
        return rc;
    }

    rc = v3_program_eval(ctx, NULL, (v3_node_t *)program);
    if (rc != V3_OK) return rc;
    ret = ctx->ret.value;

    if (ret != (v3_base_object_t *)&v3_null) {
        owner = ret;
        ret = v3_find_property(ret, v3_strobj("toString"));
        if (ret->type == V3_DATA_TYPE_FUNCTION) {
            func = (v3_function_object_t *)ret;
            ret = v3_function_apply(ctx, func, owner, NULL);
            if (ret->type == V3_DATA_TYPE_STRING) {
                str = (v3_string_object_t *)ret;
                memcpy(buf, str->value.data, str->value.length);
                buf[str->value.length] = '\0';
                printf("%s\n", buf);
            }
        }
    } else {
        printf("undefined\n");
    }

    return V3_OK;
}

v3_int_t 
v3_program_eval(v3_ctx_t *ctx, v3_frame_t *aframe, v3_node_t *anode)
{
    size_t                  i = 0;
    v3_program_node_t       *program;
    v3_node_t               *node, **nodes;
    v3_frame_t              frame;

    program = (v3_program_node_t *)anode;
    frame.global = ctx->global;
    frame.local  = ctx->global;
    frame.scope = v3_list_create(ctx->pool);
    if (frame.scope == NULL) return V3_ERROR;

    v3_list_prepend(frame.scope, ctx->global);

    frame.self   = NULL;
    frame.prev   = NULL;

    nodes = program->body->items;

    for (; i < program->body->length; i++) {
        node = nodes[i];
        CHECK_FCT(v3_evaleators[node->type](ctx, &frame, node));
    }

    return V3_OK;
}

 v3_int_t
v3_new_expr_eval(v3_ctx_t *ctx, v3_frame_t *frame, v3_node_t *node)
{
    v3_new_expr_t           *expr;
    v3_object_t             *this;
    v3_int_t                rc;

    assert(node->type == V3_SYNTAX_NEW_EXPR);

    expr = (v3_new_expr_t *)node;
    assert(expr->callee != NULL);
    CHECK_FCT(v3_evaleators[expr->callee->type](ctx, frame, expr->callee));

    if (v3_ref_base(&ctx->ret)->type != V3_DATA_TYPE_FUNCTION) {
        // TODO: return undefined;
        // TODO: throw SyntaxError
        //ctx->ret->value = v3_TypeError();
        return V3_SYNTAX_ERROR;
    }

    this = v3_object_create(ctx, 5);
    if (this == NULL) return V3_ERROR;

    if (expr->callee->type != V3_SYNTAX_CALL_EXPR) {
        return V3_SYNTAX_ERROR;
    }

    rc = v3_call_expr_eval(ctx, frame, expr->callee); 
    
    ctx->ret.value = (v3_base_object_t *)this;
    return rc;
}

 v3_int_t
v3_variable_statement_eval(v3_ctx_t *ctx, v3_frame_t *frame, v3_node_t *node)
{
    v3_variable_statement_t     *statement;
    size_t                      i;
    v3_base_object_t            *value;
    v3_variable_declarator_t    **dec;

    statement = (v3_variable_statement_t *)node;

    // if (statement->declarations == NULL) return NULL;

    dec = statement->declarations->items;

    for (i = 0; i < statement->declarations->length; i++) {
        CHECK_FCT(v3_variable_declarator_eval(ctx, frame, (v3_node_t *)dec[i]));

        value = ctx->ret.value;

        if (statement->kind.data == dec_kind_var.data) {
            CHECK_FCT(v3_obj_set(frame->local, v3_str2string(&dec[i]->id->name), value));
        } else {
            CHECK_FCT(v3_obj_set(frame->global, v3_str2string(&dec[i]->id->name), value));
        }
    }

    return V3_OK;
}

 v3_int_t 
v3_function_declaration_eval(v3_ctx_t *ctx, v3_frame_t *frame, v3_node_t *node)
{
    v3_function_node_t          *func_node;
    v3_function_object_t        *func_obj;

    func_node = (v3_function_node_t *)node;

    func_obj = v3_function_from_node(ctx, func_node, frame->scope);
    if (func_obj == NULL) return V3_ERROR;

    v3_object_set_by_str((v3_object_t *)frame->scope->elts->value, 
                            func_node->id->name.data, 
                            func_node->id->name.length, 
                            (v3_base_object_t *)func_obj);

    return V3_OK;
}

 v3_int_t 
v3_function_expr_eval(v3_ctx_t *ctx, v3_frame_t *frame, v3_node_t *node)
{
    v3_function_node_t          *func_node;
    v3_function_object_t        *func_obj;

    func_node = (v3_function_node_t *)node;

    func_obj = v3_function_from_node(ctx, func_node, frame->scope);
    if (func_obj == NULL) return V3_ERROR;

    ctx->ret.name = func_node->id->name;
    ctx->ret.value = (v3_base_object_t *)func_obj;

    return V3_OK;
}

v3_int_t 
v3_variable_declarator_eval(v3_ctx_t *ctx, v3_frame_t *frame, v3_node_t *node)
{
    v3_variable_declarator_t    *dec;

    assert(node->type == V3_SYNTAX_VARIABLE_DECLARATOR);

    dec = (v3_variable_declarator_t *)node;
    CHECK_FCT(v3_evaleators[dec->init->type](ctx, frame, dec->init));

    ctx->ret.name = dec->id->name;
    // value already seted by dec->init;

    return V3_OK;
}

/** get value of literl */
v3_int_t 
v3_literal_eval(v3_ctx_t *ctx, v3_frame_t *frame, v3_node_t *node)
{
    v3_literal_t    *literal;

    assert(node->type == V3_SYNTAX_LITERAL);
    literal = (v3_literal_t *)node;

    ctx->ret.value = literal->value;

    return V3_OK;
    //return literal->value;
}

v3_int_t
v3_identifier_eval(v3_ctx_t *ctx, v3_frame_t *frame, v3_node_t *node)
{
    v3_idetifier_t          *ident;
    v3_base_object_t        *value;
    v3_list_part_t          *part;
    v3_object_t             *scope;

    ident = (v3_idetifier_t *)node;

    for (part = frame->scope->elts; part != NULL; part = part->next) {
        scope = (v3_object_t *)part;
        value = v3_object_get_by_str(scope, ident->name.data, ident->name.length); 
        if (value != NULL) {
            //ctx->ret.value = v3_ref_create(ctx, ident->name, (v3_base_object_t*)scope);
            if (ctx->ret.value == NULL) return V3_ERROR;
            ctx->ret.name = &ident->name;
            V3_SET_RETURN(ident->name, scope);
            return V3_OK;
        }
    }

    V3_SET_RETURN(ident->name, &v3_null);

    return V3_OK;
}

v3_int_t 
v3_expr_statement_eval(v3_ctx_t *ctx, v3_frame_t *frame, v3_node_t *node)
{
    v3_expr_statement_t         *expr;
    expr = (v3_expr_statement_t *)node;

    return v3_evaleators[expr->expr->type](ctx, frame, node);
}

v3_base_object_t *
v3_find_property(v3_base_object_t *owner, v3_string_object_t *key)
{
    v3_object_t         *prototype, *object;
    v3_base_object_t    *ret;
    if (owner->type == V3_DATA_TYPE_OBJECT
        || owner->type == V3_DATA_TYPE_FUNCTION) {
        object = (v3_object_t *)owner;

        ret = v3_object_get(object, key);
        if (ret != NULL) return ret;

        prototype = (v3_object_t *)v3_object_prototype(object);
        if (prototype != NULL) {
            ret = v3_object_get(prototype, key);
            if (ret != NULL) return ret;
        }

        return v3_find_property((v3_base_object_t *)object->base.__proto__, key);
        // ret = v3_object_get(object->base.__proto__, key);
        // return ret;
    } else {
        ret = v3_find_property((v3_base_object_t *)owner->__proto__, key);
        return ret;
    }
}


v3_int_t
v3_arguments_eval(v3_ctx_t *ctx, v3_frame_t *old_frame, v3_vector_t *params)
{
    v3_vector_t     *arguments;    
    if (params == NULL || params->length == 0) {
        arguments = v3_vector_new(ctx->options, 0);
        if (arguments == NULL) return V3_ERROR;
    } else {
        for (i = 0; i< params->length; i++) {
            CHECK_FCT(V3_EVAL_NODE(old_frame, &params[i]));
        }
    }

    ctx->ret.value = arguments;
    return V3_OK;
}

v3_int_t 
v3_call_expr_eval(v3_ctx_t *ctx, v3_frame_t *old_frame, v3_node_t *node)
{
    v3_call_expr_t          *call_expr;
    v3_base_object_t        *ret;
    v3_function_object_t    *func_obj;
    v3_block_statement_t    *block;
    v3_variable_statement_t *var_stmt;
    v3_frame_t              frame;
    v3_int_t                rc;
    v3_base_object_t        *this;

    assert(node->type == V3_SYNTAX_CALL_EXPR);

    call_expr = (v3_call_expr_t *)node;
    frame.this = &v3_null;
    // TODO: get this
    CHECK_FCT(V3_EVAL_NODE(call_expr->callee, old_frame));

    if (func_obj->is_native) {
        ret = func_obj->native_func(ctx, frame);
        ctx->ret->value = ret;
        ctx->ret-name.data = NULL;
        return V3_OK;
    } else {
        // CHECK_FCT(v3_evaleators[call_expr->callee->type](frame, node));

        if (ctx->ret->value->type != &v3_undefined) {
            if (ctx->ret->value == &v3_null) {
                return V3_EREF;
                //ctx->ret->value = v3_new_eval(frame, node);
            } else {
                // TypeError

            }
        }

        func_obj = (v3_function_object_t *)ret;
        // init frame;
        frame.prev = old_frame;
        frame.call_obj = v3_object_clone(func_obj->call_obj);

        v3_list_prepend(func_obj->scope, frame.call_obj);
        frame.scope = func_obj->scope;

        ret = v3_block_statement_eval(ctx, frame, block);
        v3_list_pop(func->scope);
        return ret;
    }

    return V3_OK;
}

 v3_base_object_t *
v3_block_statement_eval(v3_ctx_t *ctx, v3_frame_t *frame, v3_node_t *node)
{
    v3_block_statement_t    *block;
    size_t                  i;

    assert(node->type == V3_SYNTAX_BLOCK_STATEMENT);
    block = node;

    for (i = 0; i < block->body->length; i++) {
        node = block->body[i];
        ret = v3_evaleators[node->type](ctx, frame, node);
        if (node->type == V3_SYNTAX_RETURN_STATEMENT) {
            return ret;
        }
    }

    return ret;
}
