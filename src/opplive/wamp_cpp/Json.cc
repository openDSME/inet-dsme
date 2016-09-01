#include "Json.h"

template<>
double convertJson<double>(Json::Value& val)
{
	return val.asDouble();
}

template<>
int convertJson<int>(Json::Value& val)
{
	return val.asInt();
}

template<>
unsigned int convertJson<unsigned int>(Json::Value& val)
{
	return val.asUInt();
}

template<>
std::string convertJson<std::string>(Json::Value& val)
{
	return val.asString();
}

template<>
bool convertJson<bool>(Json::Value& val)
{
	return val.asBool();
}

template<>
Json::Value convertJson<Json::Value>(Json::Value& val)
{
	return val;
}

