#ifndef __JSONPARSER_JSON_H_
#define __JSONPARSER_JSON_H_
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
    INVALID_STRING_ESCAPE
};

struct Json_value {
    Json_value() : type(Json_type::JSON_NULL), number(0) {}
    ~Json_value() { if (type == Json_type::JSON_STRING) str.std::string::~string(); }

    void init_str() { 
        type = Json_type::JSON_STRING; 
        new (&str) std::string();
    }

    union {
        std::string str;
        double      number;
    };
    Json_type type;
};

class Json
{
public:
    Json();
    ~Json();

    Json_state parse(Json_value* jv, const std::string& json_str);
};

} // end of JsonParser

#endif // __JSONPARSER_JSON_H_