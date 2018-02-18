#include "Json.h"

#include <cassert>

#define ASSERT_STEP(pStr, c) \
    do { \
    	assert(*pStr == c); \
    	pStr++; \
    } while (0)

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

static Json_state parse_value(Json_value *pval, const Json_Context* pjc)
{
	switch (*pjc->json_str) {
		case 'n' :  return parse_null(pval, pjc);
		case 'f' :  return parse_false(pval, pjc);
		case 't' :  return parse_true(pval, pjc);
		case '\0' : return Json_state::EXPECT_VALUE;
		default :   return Json_state::INVALID_VALUE;
	}
}

Json_state Json::parse(Json_value *pval, const std::string& json_str)
{
	Json_Context jc;

	jc.json_str = json_str.c_str();
	skip_whitespace(&jc);
	return parse_value(pval, &jc);
}

} // end of namespace JsonParser