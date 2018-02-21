#include "Json.h"

#include <cassert>
#include <cstdlib>
#include <cerrno>
#include <cmath>

#define ASSERT_STEP(pStr, c) \
    do { \
        assert(*pStr == c); \
        pStr++; \
    } while (0)

#define ISDIGIT_0TO9(c) (c >= '0' && c <= '9')
#define ISDIGIT_1TO9(c) (c >= '1' && c <= '9')

namespace JsonParser
{

Json::Json()
{
}

Json::~Json()
{
}

struct Json_Context {
    mutable const char *json_str;
    size_t json_len;
};

static void skip_whitespace(const Json_Context* pjc)
{
    const char *jstr = pjc->json_str;
    while (*jstr == ' ' || *jstr == '\t' ||
           *jstr == '\r' || *jstr == '\n') {
        jstr++;
    }
    pjc->json_str = jstr;
}

static Json_state parse_null(Json_value *pval, const Json_Context* pjc)
{
    ASSERT_STEP(pjc->json_str, 'n');
    if (*pjc->json_str++ == 'u' && 
        *pjc->json_str++ == 'l' &&
        *pjc->json_str++ == 'l') {
        pval->type = Json_type::JSON_NULL;
        return Json_state::OK;
    }
    return Json_state::INVALID_VALUE;
}

static Json_state parse_false(Json_value *pval, const Json_Context* pjc)
{
    ASSERT_STEP(pjc->json_str, 'f');
    if (*pjc->json_str++ == 'a' &&
        *pjc->json_str++ == 'l' &&
        *pjc->json_str++ == 's' &&
        *pjc->json_str++ == 'e') {
        pval->type = Json_type::JSON_FALSE;
        return Json_state::OK;
    }
    return Json_state::INVALID_VALUE;
}

static Json_state parse_true(Json_value *pval, const Json_Context* pjc)
{
    ASSERT_STEP(pjc->json_str, 't');
    if (*pjc->json_str++ == 'r' &&
        *pjc->json_str++ == 'u' &&
        *pjc->json_str++ == 'e') {
        pval->type = Json_type::JSON_TRUE;
        return Json_state::OK;
    }
    return Json_state::INVALID_VALUE;
}

static Json_state parse_number(Json_value *pval, const Json_Context* pjc)
{
    const char *p = pjc->json_str;
    double     &n = pval->number;

    if (*p == '-') p++;

    if (*p == '0') {
        p++;
    } else {
        if (!ISDIGIT_1TO9(*p)) 
            return Json_state::INVALID_VALUE;
        while (ISDIGIT_0TO9(*++p))
            ;
    }

    if (*p == '.') {
        p++;
        if (!ISDIGIT_0TO9(*p)) 
            return Json_state::INVALID_VALUE;
        while (ISDIGIT_0TO9(*++p))
            ;
    }

    if (*p == 'E' || *p == 'e') {
        p++;
        if (*p == '+' || *p == '-')
            p++;
        if (!ISDIGIT_0TO9(*p))
            return Json_state::INVALID_VALUE;
        while (ISDIGIT_0TO9(*++p))
            ;
    }

    n = std::strtod(pjc->json_str, NULL);
    if (n == HUGE_VAL || n == -HUGE_VAL)
        return Json_state::NUMBER_TOO_BIG;

    pjc->json_str = p;
    pval->type = Json_type::JSON_NUMBER;
    return Json_state::OK;
}

static Json_state parse_string(Json_value *pval, const Json_Context* pjc)
{
    ASSERT_STEP(pjc->json_str, '\"');

    const char *&p = pjc->json_str;
    std::string &s = pval->str;

    // pval->type = Json_type::JSON_STRING;
    // new (&s) std::string();
    pval->init_str();

    s.reserve(pjc->json_len);
    while (true) {
        char ch = *p++;
        switch (ch) {
            case '\"' : // end of qoutation
                return Json_state::OK;
            case '\\' : // escape char
                switch (*p++) {
                    case '\"' : s.push_back('\"'); break;
                    case '\\' : s.push_back('\\'); break;
                    case '/' :  s.push_back('/' ); break;
                    case 'b' :  s.push_back('\b'); break;
                    case 'f' :  s.push_back('\f'); break;
                    case 'n' :  s.push_back('\n'); break;
                    case 'r' :  s.push_back('\r'); break;
                    case 't' :  s.push_back('\t'); break;
                    default : return Json_state::INVALID_STRING_ESCAPE;
                }
                break;
            case '\0' : // end of string
                return Json_state::MISS_QUOTATION_MARK;
            default :
                if ((unsigned char)ch < 0x20) {
                    return Json_state::INVALID_STRING_CHAR;
                }
                s.push_back(ch);
        }
    }
}

static Json_state parse_value(Json_value *pval, const Json_Context* pjc)
{
    switch (*pjc->json_str) {
        case 'n' :  return parse_null(pval, pjc);
        case 'f' :  return parse_false(pval, pjc);
        case 't' :  return parse_true(pval, pjc);
        case '\"' : return parse_string(pval, pjc);
        case '\0' : return Json_state::EXPECT_VALUE;
        // default :   return Json_state::INVALID_VALUE;
        default :   return parse_number(pval, pjc);
    }
}

Json_state Json::parse(Json_value *pval, const std::string& json_str)
{
    Json_Context jc;
    Json_state   state;

    jc.json_str = json_str.c_str();
    jc.json_len = json_str.size() + 1;

    skip_whitespace(&jc);
    state = parse_value(pval, &jc);

    if (state == Json_state::OK) {
        skip_whitespace(&jc);
        if (*jc.json_str != '\0')
            state = Json_state::ROOT_NOT_SINGULAR;
    }
    
    return state;
}

} // end of namespace JsonParser