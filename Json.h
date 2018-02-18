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
    ROOT_NOT_SINGULAR
};

struct Json_value {
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