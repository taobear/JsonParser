#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

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

#define EXPECT_EQ_DOUBLE(expect, actual)\
    EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%lf")

#define EXPECT_EQ_STRING(expect, actual) \
    EXPECT_EQ_BASE((expect) == (actual), expect.c_str(), \
    actual.c_str(), "%s")

#define TEST_ERROR(error, jstr) \
    do {\
        Json js; \
        Json_value val; \
        EXPECT_EQ_INT(js.parse(&val, jstr), error); \
    } while (0)

#define TEST_VALUE(expect_type, jstr) \
    do { \
        Json js; \
        Json_value val; \
        EXPECT_EQ_INT(js.parse(&val, jstr), Json_state::OK); \
        EXPECT_EQ_INT(val.type, expect_type); \
    } while (0)

#define TEST_NUMBER(expect_num, jstr) \
    do { \
        Json js; \
        Json_value val; \
        EXPECT_EQ_INT(js.parse(&val, jstr), Json_state::OK); \
        EXPECT_EQ_INT(val.type, Json_type::JSON_NUMBER); \
        EXPECT_EQ_DOUBLE(expect_num, val.number); \
    } while (0)

#define TEST_STRING(expect_str,jstr) \
    do { \
        Json js; \
        Json_value val; \
        EXPECT_EQ_INT(js.parse(&val, jstr), Json_state::OK); \
        EXPECT_EQ_INT(val.type, Json_type::JSON_STRING); \
        EXPECT_EQ_STRING(std::string(expect_str), val.str); \
    } while (0)

static void test_parse_null() 
{
    // Json       js;
    // Json_value val;

    // EXPECT_EQ_INT(js.parse(&val, "null"), Json_state::OK);
    // EXPECT_EQ_INT(val.type, Json_type::JSON_NULL);
    TEST_VALUE(Json_type::JSON_NULL, "null");
}

static void test_parse_false()
{
    // Json       js;
    // Json_value val;

    // EXPECT_EQ_INT(js.parse(&val, "false"), Json_state::OK);
    // EXPECT_EQ_INT(val.type, Json_type::JSON_FALSE);
    TEST_VALUE(Json_type::JSON_FALSE, "false");
}

static void test_parse_true()
{
    // Json       js;
    // Json_value val;

    // EXPECT_EQ_INT(js.parse(&val, "true"), Json_state::OK);
    // EXPECT_EQ_INT(val.type, Json_type::JSON_TRUE);
    TEST_VALUE(Json_type::JSON_TRUE, "true");
}

static void test_parse_expect_value()
{
    // Json       js;
    // Json_value val;

    // EXPECT_EQ_INT(js.parse(&val, ""), Json_state::EXPECT_VALUE);
    // EXPECT_EQ_INT(js.parse(&val, " "), Json_state::EXPECT_VALUE);
    TEST_ERROR(Json_state::EXPECT_VALUE, "");
    TEST_ERROR(Json_state::EXPECT_VALUE, " ");
}

static void test_parse_invalid_value()
{
    // Json       js;
    // Json_value val;

    // EXPECT_EQ_INT(js.parse(&val, "nul"), Json_state::INVALID_VALUE);
    // EXPECT_EQ_INT(js.parse(&val, "?"), Json_state::INVALID_VALUE);
    TEST_ERROR(Json_state::INVALID_VALUE, "nul");
    TEST_ERROR(Json_state::INVALID_VALUE, "?");
}

static void test_parse_root_not_singular()
{
    // Json       js;
    // Json_value val;

    // EXPECT_EQ_INT(js.parse(&val, "null x"), Json_state::ROOT_NOT_SINGULAR);
    TEST_ERROR(Json_state::ROOT_NOT_SINGULAR, "null x");
}

static void test_parse_number()
{
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    /* the smallest number > 1 */
    TEST_NUMBER(1.0000000000000002, "1.0000000000000002");
    /* minimum denormal */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324");
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    /* Max subnormal double */
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    /* Min normal positive double */
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    /* Max double */
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_parse_invalid_number()
{
    TEST_ERROR(Json_state::INVALID_VALUE, "+0");
    TEST_ERROR(Json_state::INVALID_VALUE, "+1");
    TEST_ERROR(Json_state::INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(Json_state::INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(Json_state::INVALID_VALUE, "INF");
    TEST_ERROR(Json_state::INVALID_VALUE, "inf");
    TEST_ERROR(Json_state::INVALID_VALUE, "NAN");
    TEST_ERROR(Json_state::INVALID_VALUE, "nan");
}

static void test_parse_number_too_big()
{
    TEST_ERROR(Json_state::NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(Json_state::NUMBER_TOO_BIG, "-1e309");
}

static void test_parse_string()
{
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}

static void test_parse_missing_quotation_mark()
{
    TEST_ERROR(Json_state::MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(Json_state::MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invaild_string_escape()
{
#if 1
    TEST_ERROR(INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(INVALID_STRING_ESCAPE, "\"\\x12\"");
#endif
}

static void test_parse_invalid_string_char() 
{
#if 1
    TEST_ERROR(INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(INVALID_STRING_CHAR, "\"\x1F\"");
#endif
}

static void test_parse()
{
    test_parse_null();
    test_parse_false();
    test_parse_true();

    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();

    test_parse_number();
    test_parse_invalid_number();
    test_parse_number_too_big();

    test_parse_string();
    test_parse_missing_quotation_mark();
    test_parse_invaild_string_escape();
    test_parse_invalid_string_char();
}

int main(int argc, char **argv)
{
#ifdef _WINDOWS
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}