#ifndef QSON_H__
#define QSON_H__

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>  

#ifdef __cplusplus
extern "C" {
#endif

#define QSON_ASSERT(x)        assert(x)
#define QSON_ALWAYS_INLINE    static inline
#define QSON_API

#define QSON_ISDIGIT(ch)      ((ch) >= '0' && (ch) <= '9')
#define QSON_ISDIGIT1TO9(ch)  ((ch) >= '1' && (ch) <= '9')

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
	union {
		struct {char *str; size_t len;} ;
		double n;
	};
    qson_type_t type;
} qson_value_t;

enum {
    QSON_PARSE_OK = 0,
    QSON_PARSE_EXPECT_VALUE,
    QSON_PARSE_INVALID_VALUE,
    QSON_PARSE_ROOT_NOT_SINGULAR,
    QSON_PARSE_NUMBER_TOO_BIG      // http://www.cplusplus.com/reference/cstdlib/strtod/?kw=strtod
};
typedef int qson_ret_t;

typedef struct qson_context_s {
    const char *json;
} qson_context_t;

#define EXPECT_CHAR(ctx, ch) do {           \
    QSON_ASSERT(*(ctx->json) == (ch));      \
    ctx->json++;                            \
} while(0)

void qson_free(qson_value_t *v)
{
	QSON_ASSERT(v!=NULL);
	if(v->type == QSON_TYPE_STRING)
	{
		free(v->str); // c11 required.
	}
	v->type = QSON_TYPE_NULL;
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

// number = [ "-" ] int [ frac ] [ exp ]
// int = "0" / digit1-9 *digit
// frac = "." 1*digit
// exp = ("e" / "E") ["-" / "+"] 1*digit
QSON_ALWAYS_INLINE qson_ret_t qson_parse_number(qson_context_t *ctx, qson_value_t *v)
{
    const char* p = ctx->json;

    if(*p == '-') 
    {
        p++;
    }

    if(*p == '0')
    {
        p++;
    }
    else {
        if(!QSON_ISDIGIT1TO9(*p)) return QSON_PARSE_INVALID_VALUE;
        for(p++;QSON_ISDIGIT(*p);p++) 
        {
            // NOP
        }
    }

    if(*p == '.')
    {
        p++;
        if(!QSON_ISDIGIT(*p)) return QSON_PARSE_INVALID_VALUE;
        for(p++;QSON_ISDIGIT(*p);p++) {}
    }

    if(*p == 'e' || *p == 'E') {
        p++;
        if(*p == '+' || *p == '-') 
        {
            // 1e+3 is valid.
            p++;
        }
        if(!QSON_ISDIGIT(*p)) 
        {
            return QSON_PARSE_INVALID_VALUE;
        }
        for(p++; QSON_ISDIGIT(*p); p++) {}
    }
    
    errno = 0;
    v->n = strtod(ctx->json, NULL);
    if(errno == ERANGE && v->n == HUGE_VAL)
    {
        return QSON_PARSE_NUMBER_TOO_BIG;
    }

    // valid number
    ctx->json = p;
    v->type = QSON_TYPE_NUMBER;
    return QSON_PARSE_OK;
}

// QSON_ALWAYS_INLINE qson_ret_t qson_parse_literal(qson_context_t *ctx, qson_value_t *v, const char *literal, qson_type_t type)
// {}

QSON_ALWAYS_INLINE qson_ret_t qson_parse_string(qson_context_t *ctx, qson_value_t *v)
{
    EXPECT_CHAR(ctx, '\"');
	
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
        default:   return qson_parse_number(ctx, v); break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

QSON_API 
QSON_ALWAYS_INLINE qson_type_t qson_get_type(const qson_value_t *v)
{
    QSON_ASSERT(v != NULL);
    return v->type;
}

QSON_API
QSON_ALWAYS_INLINE double qson_get_number(const qson_value_t *v)
{
    QSON_ASSERT(v != NULL && v->type == QSON_TYPE_NUMBER);
    return v->n;
}

QSON_API 
QSON_ALWAYS_INLINE qson_ret_t qson_parse(qson_value_t *v, const char *json)
{
    QSON_ASSERT(v != NULL);
    int ret;
    v->type = QSON_TYPE_NULL;
    qson_context_t ctx;
    ctx.json = json;
    qson_parse_whitespace(&ctx);
    if((ret = qson_parse_value(&ctx, v)) == QSON_PARSE_OK)
    {
        qson_parse_whitespace(&ctx);
        if(*ctx.json != '\0')
        {
            ret = QSON_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

#ifdef __cplusplus
}
#endif

#endif
