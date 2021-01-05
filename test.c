#include "include/qson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int test_count = 0;
static int test_pass = 0;
static int main_ret = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format)    \
    do {                                                    \
        test_count++;                                       \
        if(equality) {                                      \
            test_pass++;                                    \
        }                                                   \
        else {                                              \
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;                                   \
        }                                                   \
    } while(0)

#define EXPECT_EQ_INT(expect, actual)      EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual)   EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")

#define TEST_ERROR(error, json)                             \           
    do {                                                    \
        qson_value_t v;                                     \
        v.type = QSON_TYPE_FALSE;                           \
        EXPECT_EQ_INT(error, qson_parse(&v, json));         \
        EXPECT_EQ_INT(QSON_TYPE_NULL, qson_get_type(&v));   \
    }while(0)

#define TEST_NUMBER(expect, json)                           \
    do {                                                    \
        qson_value_t v;                                     \
        EXPECT_EQ_INT(QSON_PARSE_OK, qson_parse(&v, json)); \
        EXPECT_EQ_INT(QSON_TYPE_NUMBER, qson_get_type(&v)); \
        EXPECT_EQ_DOUBLE(expect, qson_get_number(&v));      \
    } while(0)

static void test_parse_null()
{
    qson_value_t v;
    v.type = QSON_TYPE_TRUE;
    EXPECT_EQ_INT(QSON_PARSE_OK, qson_parse(&v, "null"));
    EXPECT_EQ_INT(QSON_TYPE_NULL, qson_get_type(&v));
}

static void test_parse_true()
{
    qson_value_t v;
    v.type = QSON_TYPE_NULL;
    EXPECT_EQ_INT(QSON_PARSE_OK, qson_parse(&v, "true"));
    EXPECT_EQ_INT(QSON_TYPE_TRUE, qson_get_type(&v));
}

static void test_parse_false()
{
    qson_value_t v;
    v.type = QSON_TYPE_NULL;
    EXPECT_EQ_INT(QSON_PARSE_OK, qson_parse(&v, "false"));
    EXPECT_EQ_INT(QSON_TYPE_FALSE, qson_get_type(&v));
}

static void test_parse_root_not_sigular() 
{
    qson_value_t v;
    v.type = QSON_TYPE_FALSE;
    EXPECT_EQ_INT(QSON_PARSE_ROOT_NOT_SINGULAR, qson_parse(&v, "null xxx"));
    EXPECT_EQ_INT(QSON_TYPE_NULL, qson_get_type(&v));
}

static void test_parse_expect_value()
{
    TEST_ERROR(QSON_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(QSON_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_number_valid()
{
    TEST_NUMBER(1.0, "1.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(1, "1.0");
    TEST_NUMBER(1.1, "1.1000000000000000001");  // ok
    TEST_NUMBER(-1.0, "-1.0");
    TEST_NUMBER(-1, "-1.0");
    TEST_NUMBER(1, "0x1");                      // ok
    TEST_NUMBER(1.3333, "1.3333");
    TEST_NUMBER(1e3, "1e3");
    TEST_NUMBER(1000, "1e3");
    TEST_NUMBER(1e3, "1000");
    TEST_NUMBER(1e-3, "1e-3");
    TEST_NUMBER(1e-3, "0.001");
    TEST_NUMBER(0.001, "1e-3");
}

static void test_parse_number_invalid()
{
    TEST_ERROR(QSON_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(QSON_PARSE_INVALID_VALUE, ".123");
    TEST_ERROR(QSON_PARSE_INVALID_VALUE, "+1.");
}

static void test_parse()
{
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_expect_value();
    test_parse_root_not_sigular();
    test_parse_number_valid();
    test_parse_number_invalid();
}

static void test_0123()
{
	qson_value_t v;
	qson_parse(&v, "0123");
}

int main(int argc, char **argv)
{
	test_0123();
//    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}
