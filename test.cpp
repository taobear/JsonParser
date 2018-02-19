#include "Json.h"
using namespace JsonParser;

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
     do { \
          test_count++; \
          if (equality) \
              test_pass++; \
          else { \
              fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", \
                  __FILE__, __LINE__, expect, actual); \
              main_ret = 1; \
          } \
     } while (0)

#define EXPECT_EQ_INT(expect, actual) \
    EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

static void test_parse_null() 
{
    Json       js;
    Json_value val;

    EXPECT_EQ_INT(js.parse(&val, "null"), Json_state::OK);
    EXPECT_EQ_INT(val.type, Json_type::JSON_NULL);
}

static void test_parse_false()
{
    Json       js;
    Json_value val;

    EXPECT_EQ_INT(js.parse(&val, "false"), Json_state::OK);
    EXPECT_EQ_INT(val.type, Json_type::JSON_FALSE);
}

static void test_parse_true()
{
    Json       js;
    Json_value val;

    EXPECT_EQ_INT(js.parse(&val, "true"), Json_state::OK);
    EXPECT_EQ_INT(val.type, Json_type::JSON_TRUE);
}

static void test_parse_expect_value()
{
    Json       js;
    Json_value val;

    EXPECT_EQ_INT(js.parse(&val, ""), Json_state::EXPECT_VALUE);
    EXPECT_EQ_INT(js.parse(&val, " "), Json_state::EXPECT_VALUE);
}

static void test_parse_invalid_value()
{
    Json       js;
    Json_value val;

    EXPECT_EQ_INT(js.parse(&val, "nul"), Json_state::INVALID_VALUE);
    EXPECT_EQ_INT(js.parse(&val, "?"), Json_state::INVALID_VALUE);
}

static void test_parse_root_not_singular()
{
    Json       js;
    Json_value val;

    EXPECT_EQ_INT(js.parse(&val, "null x"), Json_state::ROOT_NOT_SINGULAR);
}

static void test_parse()
{
    test_parse_null();
    test_parse_false();
    test_parse_true();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
}

int main(int argc, char **argv)
{
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}