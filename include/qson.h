#ifndef QSON_H__
#define QSON_H__

#include <assert.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define QSON_ASSERT(x) assert(x)
#define QSON_ALWAYS_INLINE static inline

typedef enum {
    QSON_TYPE_NULL,
    QSON_TYPE_FALSE,
    QSON_TYPE_TRUE,
    QSON_TYPE_NUMBER,
    QSON_TYPE_STRING,
    QSON_TYPE_ARRAY,
    QSON_TYPE_OBJECT
} qson_type_t;

typedef struct qson_value_s {
    qson_type_t type;
} qson_value_t;

enum {
    QSON_PARSE_OK = 0,
    QSON_PARSE_EXPECT_VALUE,
    QSON_PARSE_INVALID_VALUE,
    QSON_PARSE_ROOT_NOT_SINGULAR
};
typedef int qson_ret_t;

typedef struct qson_context_s {
    const char *json;
} qson_context_t;

#define EXPECT_CHAR(ctx, ch) do {           \
    QSON_ASSERT(*(ctx->json) == (ch));      \
    ctx->json++;                            \
} while(0)

QSON_ALWAYS_INLINE qson_type_t qson_get_type(const qson_value_t *v)
{
    QSON_ASSERT(v != NULL);
    return v->type;
}

// Eat whitespace chars like ' '/'\t'/'\n'/'\r'
// \t@param ctx
QSON_ALWAYS_INLINE void qson_parse_whitespace(qson_context_t *ctx)
{
    const char *p = ctx->json;
    while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
    {
        p++;
    }
    ctx->json = p;
}

QSON_ALWAYS_INLINE qson_ret_t qson_parse_null(qson_context_t *ctx, qson_value_t *v)
{
    EXPECT_CHAR(ctx, 'n');
    if(ctx->json[0] != 'u' || ctx->json[1] != 'l' || ctx->json[2] != 'l')
    {
        return QSON_PARSE_INVALID_VALUE;
    }
    ctx->json += 3;
    v->type = QSON_TYPE_NULL;
    return QSON_PARSE_OK;
}

QSON_ALWAYS_INLINE qson_ret_t qson_parse_false(qson_context_t *ctx, qson_value_t *v)
{
    EXPECT_CHAR(ctx, 'f');
    if(ctx->json[0] != 'a' || ctx->json[1] != 'l' || ctx->json[2] != 's' || ctx->json[3] != 'e')
    {
        return QSON_PARSE_INVALID_VALUE;
    }
    ctx->json += 4;
    v->type = QSON_TYPE_FALSE;
    return QSON_PARSE_OK;
}

QSON_ALWAYS_INLINE qson_ret_t qson_parse_true(qson_context_t *ctx, qson_value_t *v)
{
    EXPECT_CHAR(ctx, 't');
    if(ctx->json[0] != 'r' || ctx->json[1] != 'u' || ctx->json[2] != 'e')
    {
        return QSON_PARSE_INVALID_VALUE;
    }
    ctx->json += 3;
    v->type = QSON_TYPE_TRUE;
    return QSON_PARSE_OK;
}

QSON_ALWAYS_INLINE qson_ret_t qson_parse_value(qson_context_t *ctx, qson_value_t *v)
{
    switch (*ctx->json)
    {
        case 'n':  return qson_parse_null(ctx, v); break;
        case 't':  return qson_parse_true(ctx, v); break;
        case 'f':  return qson_parse_false(ctx, v); break;
        case '\0': return QSON_PARSE_EXPECT_VALUE; break;
        default:   return QSON_PARSE_INVALID_VALUE; break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
QSON_ALWAYS_INLINE qson_ret_t qson_parse(qson_value_t *v, const char *json)
{
    QSON_ASSERT(v != NULL);
    v->type = QSON_TYPE_NULL;
    qson_context_t ctx;
    ctx.json = json;
    qson_parse_whitespace(&ctx);
    return qson_parse_value(&ctx, v);
}

#ifdef __cplusplus
}
#endif

#endif