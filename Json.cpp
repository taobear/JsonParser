#include "Json.h"

#include <cassert>
#include <cstdlib>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <vector>

#define ASSERT_STEP(pStr, c) \
    do { \
        assert(*pStr == c); \
        pStr++; \
    } while (0)

#define ISDIGIT_0TO9(c) (c >= '0' && c <= '9')
#define ISDIGIT_1TO9(c) (c >= '1' && c <= '9')
#define PUTC(s, c) s.push_back(c)
namespace JsonParser
{

Json::Json()
{
}

Json::~Json()
{
}

Json_value::Json_value()
{
    str.pch = nullptr;
    str.len = 0;
    type == Json_type::JSON_NULL;
}

Json_value::~Json_value()
{
    if (type == Json_type::JSON_STRING) {
        if (str.pch != nullptr) free(str.pch);
    } else if (type == Json_type::JSON_ARRAY) {
        delete []arr.elem;
    } else if (type == Json_type::JSON_OBJECT) {
        delete []obj.mem;
    }
    type = Json_type::JSON_NULL;
}

Json_member::Json_member()
{
    key = nullptr;
    klen = 0;
}

Json_member::~Json_member()
{
    if (key != nullptr) free(key);
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

static bool parse_hex4(unsigned& u, const char *&p)
{
    u = 0;
    for (int i = 0; i < 4; ++i) {
        char ch = *p++;
        u <<= 4;
        if      (ch >= '0' && ch <= '9') u |= ch - '0';
        else if (ch >= 'A' && ch <= 'F') u |= (ch - 'A') + 10;
        else if (ch >= 'a' && ch <= 'f') u |= (ch - 'a') + 10;
        else return false;
    }
    return true;
}

static void encode_utf8(std::string &s, unsigned u)
{
    if (u <= 0x7F) {
        PUTC(s, u & 0xFF);
    } else if (u <= 0x7FF) {
        PUTC(s, 0xC0 | ((u >> 6) & 0xFF));
        PUTC(s, 0x80 | ( u       & 0x3F));
    } else if (u <= 0xFFFF) {
        PUTC(s, 0xE0 | ((u >> 12) & 0xFF));
        PUTC(s, 0x80 | ((u >>  6) & 0x3F));
        PUTC(s, 0x80 | ( u        & 0x3F));
    } else {
        assert(u <= 0x10FFFF);
        PUTC(s, 0xF0 | ((u >> 18) & 0xFF));
        PUTC(s, 0x80 | ((u >> 12) & 0x3F));
        PUTC(s, 0x80 | ((u >>  6) & 0x3F));
        PUTC(s, 0x80 | ( u        & 0x3F));
    }
}

static void set_value_raw_string(char *&raw_str, size_t &len,
                                 const std::string& s)
{
    len = s.size();
    raw_str = (char*)malloc(len + 1);
    memcpy(raw_str, s.c_str(), len);
}

static Json_state parse_raw_string(char *&raw_str, size_t &len,
                                   const Json_Context *pjc)
{
    ASSERT_STEP(pjc->json_str, '\"');

    const char *&p = pjc->json_str;
    unsigned u, u2;

    std::string s;
    s.reserve(pjc->json_len);

    while (true) {
        char ch = *p++;
        switch (ch) {
            case '\"' : // end of qoutation
                set_value_raw_string(raw_str, len, s);
                return Json_state::OK;
            case '\\' : // escape char
                switch (*p++) {
                    case '\"' : PUTC(s, '\"'); break;
                    case '\\' : PUTC(s, '\\'); break;
                    case '/' :  PUTC(s, '/') ; break;
                    case 'b' :  PUTC(s, '\b'); break;
                    case 'f' :  PUTC(s, '\f'); break;
                    case 'n' :  PUTC(s, '\n'); break;
                    case 'r' :  PUTC(s, '\r'); break;
                    case 't' :  PUTC(s, '\t'); break;
                    case 'u' :  
                        if (!parse_hex4(u, p))
                            return Json_state::INVALID_UNICODE_HEX;
                        if (u >= 0xD800 && u <= 0xDBFF) {
                            if (*p++ != '\\')
                                return Json_state::INVALID_UNICODE_SURROGATE;
                            if (*p++ != 'u')
                                return Json_state::INVALID_UNICODE_SURROGATE;
                            if (!parse_hex4(u2, p))
                                return Json_state::INVALID_UNICODE_HEX;
                            if (u2 < 0xDC00 || u2 > 0xDFFF)
                                return Json_state::INVALID_UNICODE_SURROGATE;
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        encode_utf8(s, u);
                        break;
                    
                    default : return Json_state::INVALID_STRING_ESCAPE;
                }
                break;
            case '\0' : // end of string
                return Json_state::MISS_QUOTATION_MARK;
            default :
                if ((unsigned char)ch < 0x20) {
                    return Json_state::INVALID_STRING_CHAR;
                }
                PUTC(s, ch);
        }
    }
}

static Json_state parse_string(Json_value *pval, const Json_Context *pjc)
{
    char *p = nullptr; size_t len;
    Json_state ret_state;

    ret_state = parse_raw_string(p, len, pjc);
    if (ret_state == Json_state::OK) {
        pval->str.pch = p;
        pval->str.len = len;
        pval->type = Json_type::JSON_STRING;
    } else if (p != nullptr) {
        free(p);
        p = nullptr;
    }
    return ret_state;
}

static void transfer_value_array(Json_value *pval,
                                 std::vector<Json_value *>& pv_vec)
{
    size_t size = pv_vec.size();
    pval->arr.size = size;
    pval->arr.elem = new Json_value[size];
    for (size_t i = 0; i < size; ++i) {
        memcpy(pval->arr.elem + i, pv_vec[i], sizeof(Json_value)); // shallow copy
        free(pv_vec[i]); // not delete
    }
    pval->type = Json_type::JSON_ARRAY;
}

// forward declaration
static Json_state parse_value(Json_value *pval, const Json_Context *pjc);

static Json_state parse_array(Json_value *pval, const Json_Context *pjc)
{
    ASSERT_STEP(pjc->json_str, '[');

    skip_whitespace(pjc);

    if (*pjc->json_str == ']') {
        pjc->json_str++;
        pval->type = JSON_ARRAY;
        pval->arr.size = 0;
        pval->arr.elem = NULL;
        return Json_state::OK;
    } 

    Json_state ret_state;
    std::vector<Json_value*> pv_vec;

    auto del_pv = [](const std::vector<Json_value*>& pvec) {
        for (auto & e : pvec)
            delete e;
    };

    while (true) { 
        // Json_value *pv_tmp = (Json_value*)malloc(sizeof(Json_value)); 
        Json_value *pv_tmp = new Json_value;
        pv_vec.push_back(pv_tmp);

        // parse value
        ret_state = parse_value(pv_tmp, pjc);
        if (ret_state != Json_state::OK) {
            del_pv(pv_vec);
            return ret_state;
        } 

        // parse end of array
        skip_whitespace(pjc);
        if (*pjc->json_str == ',') {
            pjc->json_str++;
            skip_whitespace(pjc);
        } else if (*pjc->json_str == ']') {
            pjc->json_str++;
            transfer_value_array(pval, pv_vec);
            return Json_state::OK;
        } else {
            del_pv(pv_vec);
            return Json_state::MISS_COMMA_OR_SQUARE_BRACKET;
        }
    }

}

static Json_state transfer_value_object(Json_value *pval,
                                        std::vector<Json_member *> pjm)
{
    size_t size = pjm.size();
    pval->obj.size = size;
    pval->obj.mem = new Json_member[size];
    for (size_t i = 0; i < size; ++i) {
        memcpy(pval->obj.mem + i, pjm[i], sizeof(Json_member)); // shallow copy
        free(pjm[i]); // not delete
        pjm[i] = nullptr;
    }
    pval->type = Json_type::JSON_OBJECT;
}

static Json_state parse_object(Json_value *pval, const Json_Context *pjc)
{
    ASSERT_STEP(pjc->json_str, '{');

    skip_whitespace(pjc);

    if (*pjc->json_str == '}') {
        pjc->json_str++;
        pval->type = JSON_OBJECT;
        pval->obj.mem = nullptr;
        pval->obj.size = 0;
        return Json_state::OK;
    }

    Json_state ret_state;
    std::vector<Json_member*> pm_vec;

    auto del_pv = [](const std::vector<Json_member*>& pvec) {
        for (auto & e : pvec)
            delete e;
    };

    while (true) {
        // Json_member *pjm = (Json_member*)(malloc(sizeof(Json_member)));
        Json_member *pm_tmp = new Json_member; // need initialize
        pm_vec.push_back(pm_tmp);

        // parse key
        if (*pjc->json_str != '"') {
            del_pv(pm_vec);
            return Json_state::MISS_KEY;
        }
        ret_state = parse_raw_string(pm_tmp->key, pm_tmp->klen, pjc);
        if (ret_state != Json_state::OK) {
            del_pv(pm_vec);
            return ret_state;
        }

        // parse comma
        skip_whitespace(pjc);
        if (*pjc->json_str != ':') {
            del_pv(pm_vec);
            return Json_state::MISS_COLON;
        }
        pjc->json_str++;

        // parse value
        skip_whitespace(pjc);
        ret_state = parse_value(&pm_tmp->val, pjc);
        if (ret_state != Json_state::OK) {
            del_pv(pm_vec);
            return ret_state;
        }

        // parse end of member
        skip_whitespace(pjc);
        if (*pjc->json_str == ',') {
            pjc->json_str++;
            skip_whitespace(pjc);
        } else if (*pjc->json_str == '}') {
            pjc->json_str++;
            transfer_value_object(pval, pm_vec);
            return Json_state::OK;
        } else {
            del_pv(pm_vec);
            return Json_state::MISS_COMMA_OR_CURLY_BRACKET;
        }
    }
}

static Json_state parse_value(Json_value *pval, const Json_Context *pjc)
{
    switch (*pjc->json_str) {
        case 'n' :  return parse_null(pval, pjc);
        case 'f' :  return parse_false(pval, pjc);
        case 't' :  return parse_true(pval, pjc);
        case '\"' : return parse_string(pval, pjc);
        case '[' :  return parse_array(pval, pjc);
        case '{' :  return parse_object(pval, pjc);
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

} // end namespace JsonParser