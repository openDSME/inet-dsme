#ifndef JSON_H_
#define JSON_H_

#include "jsoncpp/json/json_reader.h"
#include "jsoncpp/json/json_value.h"
#include "jsoncpp/json/json_writer.h"
#include <functional>
#include <vector>


typedef std::function<Json::Value(std::vector<Json::Value>)> RemoteProcedure;

template<class T>
T convertJson(Json::Value& val)
{
	return T();
}

template<>
float convertJson<float>(Json::Value& val);

template<>
int convertJson<int>(Json::Value& val);

template<>
std::string convertJson<std::string>(Json::Value& val);

template<>
bool convertJson<bool>(Json::Value& val);

template<>
Json::Value convertJson<Json::Value>(Json::Value& val);

#endif
