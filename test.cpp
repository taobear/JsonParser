#include "Json.h"

#include <cstring>
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
    EXPECT_EQ_BASE(std::abs(expect - actual) < 1e-10, expect, actual, "%lf")

#define EXPECT_EQ_STRING(expect, actual, alen) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == alen && \
                   memcmp(expect, actual, alen) == 0, expect, actual, "%s")

#define EXPECT_TRUE(actual) \
    EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")

#define EXPECT_FALSE(actual) \
    EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

#define TEST_ERROR(error, jstr) \
    do {\
        Json js; \
        Json_value val; \
        EXPECT_EQ_INT(error, js.parse(&val, jstr)); \
    } while (0)

#define TEST_VALUE(expect_type, jstr) \
    do { \
        Json js; \
        Json_value val; \
        EXPECT_EQ_INT(Json_state::OK, js.parse(&val, jstr)); \
        EXPECT_EQ_INT(expect_type, val.type); \
    } while (0)

#define TEST_NUMBER(expect_num, jstr) \
    do { \
        Json js; \
        Json_value val; \
        EXPECT_EQ_INT(Json_state::OK, js.parse(&val, jstr)); \
        EXPECT_EQ_INT(Json_type::JSON_NUMBER, val.type); \
        EXPECT_EQ_DOUBLE(expect_num, val.number); \
    } while (0)

#define TEST_STRING(expect_str,jstr) \
    do { \
        Json js; \
        Json_value val; \
        EXPECT_EQ_INT(Json_state::OK, js.parse(&val, jstr)); \
        EXPECT_EQ_INT(Json_type::JSON_STRING, val.type); \
        EXPECT_EQ_STRING(expect_str, val.str.pch, val.str.len); \
    } while (0)

#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%Iu")
#else
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")
#endif

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
    
    TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
    TEST_STRING("\x24", "\"\\u0024\""); /*Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\""); /* Cents sign U+20AC */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\""); /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\""); /* G clef sign U+1D11E */
}

static void test_parse_missing_quotation_mark()
{
    TEST_ERROR(Json_state::MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(Json_state::MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape()
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

static void test_parse_invalid_unicode_hex()
{
    TEST_ERROR(INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_ERROR(INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_ERROR(INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_ERROR(INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_ERROR(INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_ERROR(INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_ERROR(INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_ERROR(INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_ERROR(INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_ERROR(INVALID_UNICODE_HEX, "\"\\u000G\"");
    TEST_ERROR(INVALID_UNICODE_HEX, "\"\\u 123\"");
}

static void test_parse_invalid_unicode_surrogate()
{
    TEST_ERROR(INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_ERROR(INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_ERROR(INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_ERROR(INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_ERROR(INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");

}

static void test_parse_array()
{
    {
        Json js;
        Json_value val;

        EXPECT_EQ_INT(Json_state::OK, js.parse(&val, "[ ]"));
        EXPECT_EQ_INT(Json_type::JSON_ARRAY, val.type);
        EXPECT_EQ_SIZE_T(0, val.arr.size);
    }

    {
        Json js;
        Json_value val;

        EXPECT_EQ_INT(Json_state::OK,
                      js.parse(&val, "[ null , false , true , 123 , \"abc\"]"));

        auto &size = val.arr.size;
        auto &elem = val.arr.elem;

        EXPECT_EQ_INT(Json_type::JSON_ARRAY, val.type);
        EXPECT_EQ_SIZE_T(5, size);

        EXPECT_EQ_INT(Json_type::JSON_NULL,   elem[0].type);
        EXPECT_EQ_INT(Json_type::JSON_FALSE,  elem[1].type);
        EXPECT_EQ_INT(Json_type::JSON_TRUE,   elem[2].type);
        EXPECT_EQ_INT(Json_type::JSON_NUMBER, elem[3].type);
        EXPECT_EQ_INT(Json_type::JSON_STRING, elem[4].type);

        EXPECT_EQ_DOUBLE(123.0, elem[3].number);
        EXPECT_EQ_STRING("abc", elem[4].str.pch, 3);
    }

    {
        Json js;
        Json_value val;

        EXPECT_EQ_INT(Json_state::OK,
                      js.parse(&val, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));

        auto &lv1_size = val.arr.size;
        auto &lv1_elem = val.arr.elem;

        EXPECT_EQ_INT(Json_type::JSON_ARRAY, val.type);
        EXPECT_EQ_SIZE_T(4, lv1_size);

        for (size_t i = 0; i < 4; ++i) {
            auto &lv2_size = lv1_elem[i].arr.size;
            auto &lv2_elem = lv1_elem[i].arr.elem;

            EXPECT_EQ_INT(Json_type::JSON_ARRAY, lv1_elem[i].type);
            EXPECT_EQ_SIZE_T(i, lv2_size);

            for (size_t j = 0; j < i; ++j) {
                EXPECT_EQ_INT(Json_type::JSON_NUMBER, lv2_elem[j].type);
                EXPECT_EQ_DOUBLE((double)j, lv2_elem[j].number);
            }
        }
    }
}

static void test_parse_miss_comma_or_square_bracket()
{
    TEST_ERROR(Json_state::MISS_COMMA_OR_SQUARE_BRACKET, "[1");
    TEST_ERROR(Json_state::MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
    TEST_ERROR(Json_state::MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
    TEST_ERROR(Json_state::MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

static void test_parse_object()
{
    {
        Json js;
        Json_value val;

        EXPECT_EQ_INT(Json_state::OK, js.parse(&val, " { } "));
        EXPECT_EQ_INT(Json_type::JSON_OBJECT, val.type);
        EXPECT_EQ_SIZE_T(0, val.obj.size);
    }

#if 1
    {
        Json js;
        Json_value val;

        EXPECT_EQ_INT(Json_state::OK, js.parse(&val, 
            " { "
            "\"n\" : null , "
            "\"f\" : false , "
            "\"t\" : true , "
            "\"i\" : 123 , "
            "\"s\" : \"abc\", "
            "\"a\" : [ 1, 2, 3 ] , "
            "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
            " } "
        ));

        auto &lv1_size = val.obj.size;
        auto &lv1_mem = val.obj.mem;

        EXPECT_EQ_INT(Json_type::JSON_OBJECT, val.type);
        EXPECT_EQ_SIZE_T(7, val.obj.size);

        EXPECT_EQ_STRING("n", lv1_mem[0].key, lv1_mem[0].klen);
        EXPECT_EQ_INT(Json_type::JSON_NULL, lv1_mem[0].val.type);

        EXPECT_EQ_STRING("f", lv1_mem[1].key, lv1_mem[1].klen);
        EXPECT_EQ_INT(Json_type::JSON_FALSE, lv1_mem[1].val.type);

        EXPECT_EQ_STRING("t", lv1_mem[2].key, lv1_mem[2].klen);
        EXPECT_EQ_INT(Json_type::JSON_TRUE, lv1_mem[2].val.type);

        EXPECT_EQ_STRING("i", lv1_mem[3].key, lv1_mem[3].klen);
        EXPECT_EQ_INT(Json_type::JSON_NUMBER, lv1_mem[3].val.type);
        EXPECT_EQ_DOUBLE(123.0, lv1_mem[3].val.number);

        EXPECT_EQ_STRING("s", lv1_mem[4].key, lv1_mem[4].klen);
        EXPECT_EQ_INT(Json_type::JSON_STRING, lv1_mem[4].val.type);
        EXPECT_EQ_STRING("abc", lv1_mem[4].val.str.pch, lv1_mem[4].val.str.len);

        EXPECT_EQ_STRING("a", lv1_mem[5].key, lv1_mem[5].klen);
        {
            auto &lv2_type = lv1_mem[5].val.type;
            auto &lv2_size = lv1_mem[5].val.arr.size;
            auto &lv2_elem = lv1_mem[5].val.arr.elem;

            EXPECT_EQ_INT(Json_type::JSON_ARRAY, lv2_type);
            EXPECT_EQ_SIZE_T(3, lv2_size);

            for (size_t i = 0; i < 3; ++i) {
                EXPECT_EQ_INT(Json_type::JSON_NUMBER, lv2_elem[i].type);
                EXPECT_EQ_DOUBLE(i + 1.0, lv2_elem[i].number);
            }
        }

        EXPECT_EQ_STRING("o", lv1_mem[6].key, lv1_mem[6].klen);
        {
            auto &lv2_type = lv1_mem[6].val.type;
            auto &lv2_size = lv1_mem[6].val.obj.size;
            auto &lv2_mem  = lv1_mem[6].val.obj.mem;

            EXPECT_EQ_INT(Json_type::JSON_OBJECT, lv2_type);
            EXPECT_EQ_SIZE_T(3, lv2_size);

            for (size_t i = 0; i < 3; ++i) {
                // key
                EXPECT_TRUE('1' + i == lv2_mem[i].key[0]);
                EXPECT_EQ_SIZE_T(1, lv2_mem[i].klen);

                // number
                EXPECT_EQ_INT(Json_type::JSON_NUMBER, lv2_mem[i].val.type);
                EXPECT_EQ_DOUBLE(i + 1.0, lv2_mem[i].val.number);
            }
        }
    }
#endif
}

static void test_parse_miss_key()
{
#if 1
    TEST_ERROR(Json_state::MISS_KEY, "{:1,");
    TEST_ERROR(Json_state::MISS_KEY, "{1:1,");
    TEST_ERROR(Json_state::MISS_KEY, "{true:1,");
    TEST_ERROR(Json_state::MISS_KEY, "{false:1,");
    TEST_ERROR(Json_state::MISS_KEY, "{null:1,");
    TEST_ERROR(Json_state::MISS_KEY, "{[]:1,");
    TEST_ERROR(Json_state::MISS_KEY, "{{}:1,");
    TEST_ERROR(Json_state::MISS_KEY, "{\"a\":1,");
#endif
}

static void test_parse_miss_colon()
{
#if 1
    TEST_ERROR(Json_state::MISS_COLON, "{\"a\"}");
    TEST_ERROR(Json_state::MISS_COLON, "{\"a\",\"b\"}");
#endif
}

static void test_parse_miss_comma_or_curly_bracket()
{
#if 1
    TEST_ERROR(Json_state::MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
    TEST_ERROR(Json_state::MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
    TEST_ERROR(Json_state::MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
    TEST_ERROR(Json_state::MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
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
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();
    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();

    test_parse_array();
    test_parse_miss_comma_or_square_bracket();

    test_parse_object();
    test_parse_miss_key();
    test_parse_miss_colon();
    test_parse_miss_comma_or_curly_bracket();
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