#ifndef __JSONPARSER_JSON_H_
#define __JSONPARSER_JSON_H_
#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <string>

namespace JsonParser
{

enum Json_type {
    JSON_NULL,
    JSON_FALSE,
    JSON_TRUE,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
};

enum Json_state {
    OK = 0,
    EXPECT_VALUE,
    INVALID_VALUE,
    NUMBER_TOO_BIG,
    ROOT_NOT_SINGULAR,
    MISS_QUOTATION_MARK,
    INVALID_STRING_CHAR,
    INVALID_STRING_ESCAPE,
    INVALID_UNICODE_HEX,
    INVALID_UNICODE_SURROGATE,
    MISS_COMMA_OR_SQUARE_BRACKET,
    MISS_KEY,
    MISS_COLON,
    MISS_COMMA_OR_CURLY_BRACKET
};

struct Json_member;

struct Json_value {
    Json_value();
    ~Json_value();

    union {
        struct { Json_member *mem; size_t size; } obj;
        struct { Json_value *elem; size_t size; } arr;
        struct { char *pch; size_t len; } str;
        double number;
    };
    Json_type type;
};

struct Json_member {
    Json_member();
    ~Json_member();

    char *key; size_t klen;
    Json_value val;
};

class Json
{
public:
    Json();
    ~Json();

    Json_state parse(Json_value* jv, const std::string& json_str);
    void stringify(std::string& json_str, const Json_value* jv);
};

} // end of JsonParser

#endif // __JSONPARSER_JSON_H_